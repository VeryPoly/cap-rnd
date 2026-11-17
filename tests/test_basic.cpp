#include "../rnd.h"
#include "../rndCli.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

// Script storage belongs to tests. The engine accepts a general roll source.
class ScriptedSimulator : public Simulator
{
public:
    ScriptedSimulator(int prior, int costOffset, bool keepGoing, unsigned int seed)
        : ScriptedSimulator(prior, costOffset, keepGoing, seed, std::make_shared<std::queue<int>>())
    {
    }

    void forceNextRoll(int value)
    {
        rolls->push(value);
    }

private:
    ScriptedSimulator(int prior, int costOffset, bool keepGoing, unsigned int seed,
                      std::shared_ptr<std::queue<int>> scripted)
        : Simulator(prior, costOffset, keepGoing, seed,
                    [scripted]()
                    {
                        if (scripted->empty())
                            throw std::logic_error("Scripted rolls exhausted.");
                        const int value = scripted->front();
                        scripted->pop();
                        return value;
                    }),
          rolls(std::move(scripted))
    {
    }

    std::shared_ptr<std::queue<int>> rolls;
};

static void testConstructor()
{
    Simulator sim(0, 0, false);

    assert(sim.getGrandTotalCost() == 0);
    assert(sim.getSuccesses() == 0);
    assert(sim.getFaults() == 0);
    assert(sim.getFails() == 0);
}

static void testReset()
{
    Simulator sim(0, 0, false);

    sim.processRoll(14);
    sim.processRoll(6);
    sim.processRoll(23);
    (void)sim.finalizeAttempt();
    sim.reset();

    assert(sim.getScaledCost() == 0);
    assert(sim.getSuccesses() == 0);
    assert(sim.getFaults() == 0);
    assert(sim.getFails() == 0);
}

static void testPriorIncrement()
{
    Simulator incremented(0, 0, false, 42);
    Simulator expected(7, 0, false, 42);
    for (int i = 0; i < 20; ++i)
        incremented.incPrior();
    (void)incremented.roll();
    (void)expected.roll();
    assert(incremented.getLastRoll() == expected.getLastRoll());
}
// prior/costOffset aren't directly readable, so clamping is verified by
// comparing an out-of-range constructor against the boundary value it
// should clamp to: same seed => same rng draws => identical observable
// state if (and only if) both actually clamp to the same value.
static void testConstructorClampsPriorToMax()
{
    Simulator simClamped(1000000, 0, false, 42);
    Simulator simExplicit(Simulator::PRIOR_MAX, 0, false, 42);

    bool completeClamped = simClamped.roll();
    bool completeExplicit = simExplicit.roll();

    assert(simClamped.getLastRoll() == simExplicit.getLastRoll());
    assert(simClamped.getSuccesses() == simExplicit.getSuccesses());
    assert(simClamped.getFails() == simExplicit.getFails());
    assert(completeClamped == completeExplicit);
}

static void testConstructorClampsPriorToMin()
{
    Simulator simClamped(-1000000, 0, false, 42);
    Simulator simExplicit(Simulator::PRIOR_MIN, 0, false, 42);

    bool completeClamped = simClamped.roll();
    bool completeExplicit = simExplicit.roll();

    assert(simClamped.getLastRoll() == simExplicit.getLastRoll());
    assert(simClamped.getSuccesses() == simExplicit.getSuccesses());
    assert(simClamped.getFails() == simExplicit.getFails());
    assert(completeClamped == completeExplicit);
}

static void testConstructorClampsCostOffsetToMax()
{
    Simulator simClamped(0, 1000000, false, 42);
    Simulator simExplicit(0, Simulator::COST_OFFSET_MAX, false, 42);

    simClamped.processRoll(10);
    simExplicit.processRoll(10);

    assert(simClamped.getScaledCost() == simExplicit.getScaledCost());
    assert(simClamped.getScaledCost() > 0);
}

static void testConstructorClampsCostOffsetToMin()
{
    Simulator simClamped(0, -1000000, false, 42);
    Simulator simExplicit(0, Simulator::COST_OFFSET_MIN, false, 42);

    simClamped.processRoll(10);
    simExplicit.processRoll(10);

    assert(simClamped.getScaledCost() == simExplicit.getScaledCost());
}

static void testMultipleResets()
{
    Simulator sim(0, 0, false);

    for (int i = 0; i < 10; ++i)
    {
        sim.reset();

        assert(sim.getSuccesses() == 0);
        assert(sim.getFaults() == 0);
        assert(sim.getFails() == 0);
    }
}

struct RollBoundaryCase
{
    int rollValue;
    int expectedSuccesses;
    int expectedFaults;
    int expectedFails;
};

static const RollBoundaryCase ROLL_BOUNDARY_CASES[] = {
    {23, 3, 0, 0},  // PERFECT_SUCCESS
    {18, 1, 0, 0},  // SUCCESS lower boundary
    {22, 1, 0, 0},  // SUCCESS upper boundary (just below PERFECT_SUCCESS)
    {14, 1, 1, 0},  // SUCCESS_WITH_FAULT lower boundary
    {17, 1, 1, 0},  // SUCCESS_WITH_FAULT upper boundary
    {7, 0, 0, 0},   // neutral lower boundary
    {13, 0, 0, 0},  // neutral upper boundary
};

static void testProcessRollBoundaries()
{
    for (const RollBoundaryCase& c : ROLL_BOUNDARY_CASES)
    {
        Simulator sim(0, 0, false, 1);
        sim.processRoll(c.rollValue);

        assert(sim.getSuccesses() == c.expectedSuccesses);
        assert(sim.getFaults() == c.expectedFaults);
        assert(sim.getFails() == c.expectedFails);
    }
}

static void testProcessRollPerfectSuccessCapsRatherThanAdds()
{
    Simulator sim(0, 0, false, 1);

    sim.processRoll(18);  // successes = 1
    sim.processRoll(23);  // PERFECT_SUCCESS sets successes = 3, not 1 + 3

    assert(sim.getSuccesses() == 3);
}

static void testProcessRollCriticalFailure()
{
    Simulator sim(0, 0, false, 1);

    sim.processRoll(6);
    assert(sim.getFails() == 1);

    sim.processRoll(1);
    assert(sim.getFails() == 2);
}

static void testProcessRollAccumulatesToThreshold()
{
    Simulator sim(0, 0, false, 1);

    sim.processRoll(18);
    sim.processRoll(18);
    sim.processRoll(18);

    assert(sim.getSuccesses() == 3);
}

static void testRollReturnsTrueOnceSuccessThresholdReached()
{
    Simulator sim(0, 0, false, 1);

    sim.processRoll(18);
    sim.processRoll(18);
    sim.processRoll(18);
    assert(sim.getSuccesses() == 3);

    bool complete = sim.roll();
    assert(complete == true);
}

static void testRollReturnsTrueOnceFailThresholdReached()
{
    Simulator sim(0, 0, false, 1);

    sim.processRoll(6);
    sim.processRoll(6);
    sim.processRoll(6);
    assert(sim.getFails() == 3);

    bool complete = sim.roll();
    assert(complete == true);
}

struct CostOutcomeCase
{
    int rollValue;
    bool keepGoing;
    Simulator::CostOutcome expectedOutcome;
    bool expectGrandTotalPositive;
};

static const CostOutcomeCase COST_OUTCOME_CASES[] = {
    {18, false, Simulator::CostOutcome::Success, true},
    {14, false, Simulator::CostOutcome::RetrySuccess, false},
    {6, true, Simulator::CostOutcome::RetryFailure, false},
    {6, false, Simulator::CostOutcome::GiveUpFailure, true},
};

static void testCostOutcomes()
{
    for (const CostOutcomeCase& c : COST_OUTCOME_CASES)
    {
        Simulator sim(0, 0, c.keepGoing, 1);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);

        Simulator::CostOutcome outcome = sim.finalizeAttempt();
        assert(outcome == c.expectedOutcome);
        assert((sim.getGrandTotalCost() > 0) == c.expectGrandTotalPositive);
    }
}

// Every COST_OUTCOME_CASES row drives faults>0 via SUCCESS_WITH_FAULT
// rolls; this covers the other side of retry's "faults > 0 || fails > 0" -
// a crit failure earlier in the same attempt, followed by enough plain
// successes to still reach the success threshold.
static void testCostOutcomeRetrySuccessViaFailsNotFaults()
{
    Simulator sim(0, 0, false, 1);

    sim.processRoll(6);   // fails = 1
    sim.processRoll(18);  // successes = 1
    sim.processRoll(18);  // successes = 2
    sim.processRoll(18);  // successes = 3

    Simulator::CostOutcome outcome = sim.finalizeAttempt();
    assert(outcome == Simulator::CostOutcome::RetrySuccess);
}

static void testPrintCostOutcomes()
{
    std::ostringstream output;
    for (const CostOutcomeCase& c : COST_OUTCOME_CASES)
    {
        Simulator sim(0, 0, c.keepGoing, 1);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);

        Simulator::CostOutcome outcome = sim.finalizeAttempt();
        output.str("");
        printCost(sim, outcome, output);
        assert(outcome == c.expectedOutcome);
        assert(output.str().find("Total cost: ") != std::string::npos);
        const bool failed = outcome == Simulator::CostOutcome::RetryFailure ||
                            outcome == Simulator::CostOutcome::GiveUpFailure;
        assert(output.str().find(failed ? " failed." : " success") != std::string::npos);
    }
}

static void testRunRollSequenceReturnsTrueWhenAlreadyAtThreshold()
{
    std::ostringstream output;
    Simulator sim(0, 0, false, 1);

    sim.processRoll(18);
    sim.processRoll(18);
    sim.processRoll(18);
    assert(sim.getSuccesses() == 3);

    const auto complete = runRollSequence(sim, output);
    assert(complete == RunResult::Completed);
}

static void testRunRollSequenceExhaustsSafetyLimit()
{
    std::ostringstream output;
    ScriptedSimulator sim(0, 0, false, 1);

    // Neutral rolls exercise the limit without completing the attempt.
    for (int i = 0; i < 1000; ++i)
    {
        sim.forceNextRoll(10);
    }

    const auto complete = runRollSequence(sim, output);
    assert(complete == RunResult::RollLimitExceeded);
}

static void testPromptIntParsesValidInteger()
{
    std::ostringstream output;
    std::istringstream input("42\n");

    int value = 0;
    promptInt(value, false, input, output);

    assert(value == 42);
}

static void testPromptIntRetriesOnInvalidInputThenAcceptsValid()
{
    std::ostringstream output;
    std::istringstream input("abc\n7\n");

    int value = 0;
    promptInt(value, false, input, output);

    assert(value == 7);
}

static void testPromptIntRetriesUntilAboveZero()
{
    std::ostringstream output;
    std::istringstream input("-5\n0\n3\n");

    int value = 0;
    promptInt(value, true, input, output);

    assert(value == 3);
}

static void testPromptIntEofLeavesValueUntouched()
{
    std::ostringstream output;
    std::istringstream input("");

    int value = 99;
    promptInt(value, false, input, output);

    assert(value == 99);
}

static void testPromptBoolParsesYesNoCaseInsensitive()
{
    std::ostringstream output;
    {
        std::istringstream input("Y\n");
        bool result = false;
        promptBool(result, input, output);
        assert(result == true);
    }
    {
        std::istringstream input("N\n");
        bool result = true;
        promptBool(result, input, output);
        assert(result == false);
    }
}

static void testPromptBoolRetriesOnInvalidThenAccepts()
{
    std::ostringstream output;
    std::istringstream input("q\ny\n");

    bool result = false;
    promptBool(result, input, output);

    assert(result == true);
}

static void testPromptBoolEofDefaultsFalse()
{
    std::ostringstream output;
    std::istringstream input("");

    bool result = true;
    promptBool(result, input, output);

    assert(result == false);
}

static void testSimConfigDefaultValues()
{
    // Guards against the bug this fixed: promptInt() only assigns on a
    // successful read, so if stdin hits EOF before any input at all,
    // these fields must already be at a safe, defined value.
    SimConfig config;

    assert(config.prior == 0);
    assert(config.costOffset == 0);
    assert(config.numIterations == 0);
    assert(config.keepGoing == false);
}

static void testRunSessionZeroRollsCompletesImmediately()
{
    std::ostringstream errors;
    std::ostringstream output;
    SimConfig config;

    const auto completed = runSession(config, output, errors);
    assert(completed == RunResult::Completed);
}

static void testRunIterationCompletesWithoutHanging()
{
    std::ostringstream errors;
    std::ostringstream output;
    ScriptedSimulator sim(0, 0, true, 1);
    sim.forceNextRoll(23);

    const auto completed = runIteration(sim, output, errors);

    assert(sim.getSuccesses() == 0);
    assert(sim.getFails() == 0);
    assert(completed == RunResult::Completed);
}

static void testRunIterationExhaustsRetryLimit()
{
    std::ostringstream errors;
    std::ostringstream output;
    ScriptedSimulator sim(0, 0, false, 1);

    // Force 101 attempts requiring retries to exhaust the 100-retry limit.

    for (int i = 0; i < 101; ++i)
    {
        sim.forceNextRoll(14);
        sim.forceNextRoll(14);
        sim.forceNextRoll(14);
    }

    const auto completed = runIteration(sim, output, errors);
    assert(completed == RunResult::RetryLimitExceeded);
}

// The exhaustion path (testRunIterationExhaustsRetryLimit) is already
// deterministic; the ordinary "fails once, retries, then succeeds" path
// was previously only exercised by real (non-deterministic) dice.
static void testRunIterationSucceedsAfterOneRetry()
{
    std::ostringstream errors;
    std::ostringstream output;
    ScriptedSimulator sim(0, 0, false, 1);

    sim.forceNextRoll(14);  // attempt 1: RetrySuccess -> retry
    sim.forceNextRoll(14);
    sim.forceNextRoll(14);

    sim.forceNextRoll(18);  // attempt 2: clean Success -> done
    sim.forceNextRoll(18);
    sim.forceNextRoll(18);

    const auto completed = runIteration(sim, output, errors);
    assert(completed == RunResult::Completed);
}

static void testFinalizationAndReset()
{
    std::ostringstream output;
    Simulator sim(0, 0, false, 42);
    bool rejected = false;
    try
    {
        (void)sim.finalizeAttempt();
    }
    catch (const std::logic_error&)
    {
        rejected = true;
    }
    assert(rejected);
    sim.processRoll(23);
    const auto outcome = sim.finalizeAttempt();
    const auto total = sim.getGrandTotalCost();
    const auto repeatedOutcome = sim.finalizeAttempt();
    assert(repeatedOutcome == outcome);
    assert(sim.getGrandTotalCost() == total);
    printCost(sim, outcome, output);
    printCost(sim, outcome, output);
    assert(sim.getGrandTotalCost() == total);
    rejected = false;
    try
    {
        sim.processRoll(18);
    }
    catch (const std::logic_error&)
    {
        rejected = true;
    }
    assert(rejected);
    sim.reset();
    assert(sim.getScaledCost() == 0);
    assert(sim.getGrandTotalCost() == total);
    sim.processRoll(23);
    (void)sim.finalizeAttempt();
    assert(sim.getGrandTotalCost() > total);
}

static void testInputValidationAndCancellation()
{
    std::ostringstream output;
    {
        std::istringstream input("3.5\n999999999999999999999\n7\n");
        int value = 99;
        const bool accepted = promptInt(value, true, input, output);
        assert(accepted && value == 7);
    }
    {
        std::istringstream input("-1\n");
        int value = 99;
        const bool accepted = promptInt(value, true, input, output);
        assert(!accepted && value == 99);
    }
    {
        std::istringstream input("yes\ny\nn\n");
        bool value = false;
        const bool first = promptBool(value, input, output);
        assert(first && value);
        const bool second = promptBool(value, input, output);
        assert(second && !value);
    }
    {
        std::istringstream input("0\n0\n");
        const auto config = promptConfig(input, output);
        assert(!config);
    }
}

static void testSuccessOnLastAllowedRetry()
{
    std::ostringstream errors;
    std::ostringstream output;
    ScriptedSimulator sim(0, 0, true, 42);
    for (int i = 0; i < 100; ++i)
    {
        sim.forceNextRoll(6);
        sim.forceNextRoll(6);
        sim.forceNextRoll(6);
    }
    sim.forceNextRoll(23);
    const auto completed = runIteration(sim, output, errors);
    assert(completed == RunResult::Completed);
    assert(sim.getGrandTotalCost() > 0);
}

static void testCompletedSequencePrintsNoExtraRoll()
{
    std::ostringstream output;
    Simulator sim(0, 0, false, 42);
    sim.processRoll(23);
    assert(sim.getLastRoll() == 23);
    const auto cost = sim.getScaledCost();
    const auto completed = runRollSequence(sim, output);
    assert(completed == RunResult::Completed);
    assert(output.str().empty());
    assert(sim.getScaledCost() == cost);
}

static void testImpossibleSessionIsRejected()
{
    std::ostringstream errors;
    std::ostringstream output;
    SimConfig config;
    config.prior = -3;
    config.numIterations = 1;
    config.keepGoing = true;
    const auto completed = runSession(config, output, errors);
    assert(completed == RunResult::InvalidConfig);
    Simulator impossible(-3, 0, true, 42);
    Simulator possible(-2, 0, true, 42);
    assert(!impossible.canCompleteCleanly());
    assert(possible.canCompleteCleanly());
}

static void testSessionUsesProvidedStreams()
{
    std::istringstream input("20\n-3\n2\nn\n");
    std::ostringstream output;
    std::ostringstream errors;
    const auto config = promptConfig(input, output);
    assert(config);
    assert(config->numIterations == 2);
    const auto result = runSession(*config, output, errors);
    assert(result == RunResult::Completed);
    assert(errors.str().empty());
    assert(output.str().find("Grand total cost across all iterations: ") != std::string::npos);
}

static void testIterationPropagatesRollLimit()
{
    ScriptedSimulator sim(0, 0, false, 42);
    for (int i = 0; i < 1000; ++i)
        sim.forceNextRoll(10);
    std::ostringstream output;
    std::ostringstream errors;
    const auto result = runIteration(sim, output, errors);
    assert(result == RunResult::RollLimitExceeded);
    assert(errors.str().find("Safety limit exceeded.") != std::string::npos);
}

int main()
{
    testSessionUsesProvidedStreams();
    testIterationPropagatesRollLimit();
    testSuccessOnLastAllowedRetry();
    testCompletedSequencePrintsNoExtraRoll();
    testImpossibleSessionIsRejected();
    testFinalizationAndReset();
    testInputValidationAndCancellation();
    testConstructor();
    testReset();
    testPriorIncrement();
    testConstructorClampsPriorToMax();
    testConstructorClampsPriorToMin();
    testConstructorClampsCostOffsetToMax();
    testConstructorClampsCostOffsetToMin();
    testMultipleResets();

    testProcessRollBoundaries();
    testProcessRollPerfectSuccessCapsRatherThanAdds();
    testProcessRollCriticalFailure();
    testProcessRollAccumulatesToThreshold();

    testRollReturnsTrueOnceSuccessThresholdReached();
    testRollReturnsTrueOnceFailThresholdReached();

    testCostOutcomes();
    testCostOutcomeRetrySuccessViaFailsNotFaults();
    testPrintCostOutcomes();

    testRunRollSequenceReturnsTrueWhenAlreadyAtThreshold();
    testRunRollSequenceExhaustsSafetyLimit();

    testPromptIntParsesValidInteger();
    testPromptIntRetriesOnInvalidInputThenAcceptsValid();
    testPromptIntRetriesUntilAboveZero();
    testPromptIntEofLeavesValueUntouched();
    testPromptBoolParsesYesNoCaseInsensitive();
    testPromptBoolRetriesOnInvalidThenAccepts();
    testPromptBoolEofDefaultsFalse();

    testSimConfigDefaultValues();
    testRunSessionZeroRollsCompletesImmediately();
    testRunIterationCompletesWithoutHanging();
    testRunIterationSucceedsAfterOneRetry();
    testRunIterationExhaustsRetryLimit();

    std::cout << "All tests passed.\n";

    return 0;
}
