#include "../rnd.h"
#include "../rndCli.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

static void testConstructor()
{
    rnd sim(0, 0, false);

    assert(sim.getGrandTotalCost() == 0);
    assert(sim.getSuccesses() == 0);
    assert(sim.getFaults() == 0);
    assert(sim.getFails() == 0);
}

static void testReset()
{
    rnd sim(0, 0, false);

    sim.reset();

    assert(sim.getSuccesses() == 0);
    assert(sim.getFaults() == 0);
    assert(sim.getFails() == 0);
}

static void testPriorIncrement()
{
    rnd sim(0, 0, false);

    for (int i = 0; i < 20; ++i)
    {
        sim.incPrior();
    }

    // We cannot directly inspect prior,
    // but this verifies repeated calls do not crash.
    assert(sim.getGrandTotalCost() == 0);
}

// prior/costOffset aren't directly readable, so clamping is verified by
// comparing an out-of-range constructor against the boundary value it
// should clamp to: same seed => same rng draws => identical observable
// state if (and only if) both actually clamp to the same value.
static void testConstructorClampsPriorToMax()
{
    rnd simClamped(1000000, 0, false, 42);
    rnd simExplicit(rnd::PRIOR_MAX, 0, false, 42);

    bool completeClamped = simClamped.roll();
    bool completeExplicit = simExplicit.roll();

    assert(simClamped.getLastRoll() == simExplicit.getLastRoll());
    assert(simClamped.getSuccesses() == simExplicit.getSuccesses());
    assert(simClamped.getFails() == simExplicit.getFails());
    assert(completeClamped == completeExplicit);
}

static void testConstructorClampsPriorToMin()
{
    rnd simClamped(-1000000, 0, false, 42);
    rnd simExplicit(rnd::PRIOR_MIN, 0, false, 42);

    bool completeClamped = simClamped.roll();
    bool completeExplicit = simExplicit.roll();

    assert(simClamped.getLastRoll() == simExplicit.getLastRoll());
    assert(simClamped.getSuccesses() == simExplicit.getSuccesses());
    assert(simClamped.getFails() == simExplicit.getFails());
    assert(completeClamped == completeExplicit);
}

static void testConstructorClampsCostOffsetToMax()
{
    rnd simClamped(0, 1000000, false, 42);
    rnd simExplicit(0, rnd::COST_OFFSET_MAX, false, 42);

    simClamped.processRoll(10);
    simExplicit.processRoll(10);

    assert(simClamped.getScaledCost() == simExplicit.getScaledCost());
    assert(simClamped.getScaledCost() > 0);
}

static void testConstructorClampsCostOffsetToMin()
{
    rnd simClamped(0, -1000000, false, 42);
    rnd simExplicit(0, rnd::COST_OFFSET_MIN, false, 42);

    simClamped.processRoll(10);
    simExplicit.processRoll(10);

    assert(simClamped.getScaledCost() == simExplicit.getScaledCost());
}

static void testMultipleResets()
{
    rnd sim(0, 0, false);

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
        rnd sim(0, 0, false, 1);
        sim.processRoll(c.rollValue);

        assert(sim.getSuccesses() == c.expectedSuccesses);
        assert(sim.getFaults() == c.expectedFaults);
        assert(sim.getFails() == c.expectedFails);
    }
}

static void testProcessRollPerfectSuccessCapsRatherThanAdds()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(18);  // successes = 1
    sim.processRoll(23);  // PERFECT_SUCCESS sets successes = 3, not 1 + 3

    assert(sim.getSuccesses() == 3);
}

static void testProcessRollCriticalFailure()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(6);
    assert(sim.getFails() == 1);

    sim.processRoll(1);
    assert(sim.getFails() == 2);
}

static void testProcessRollAccumulatesToThreshold()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(18);
    sim.processRoll(18);
    sim.processRoll(18);

    assert(sim.getSuccesses() == 3);
}

static void testRollReturnsTrueOnceSuccessThresholdReached()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(18);
    sim.processRoll(18);
    sim.processRoll(18);
    assert(sim.getSuccesses() == 3);

    bool complete = sim.roll();
    assert(complete == true);
}

static void testRollReturnsTrueOnceFailThresholdReached()
{
    rnd sim(0, 0, false, 1);

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
    rnd::CostOutcome expectedOutcome;
    bool expectGrandTotalPositive;
};

static const CostOutcomeCase COST_OUTCOME_CASES[] = {
    {18, false, rnd::CostOutcome::Success, true},
    {14, false, rnd::CostOutcome::RetrySuccess, false},
    {6, true, rnd::CostOutcome::RetryFailure, false},
    {6, false, rnd::CostOutcome::GiveUpFailure, true},
};

static void testCostOutcomes()
{
    for (const CostOutcomeCase& c : COST_OUTCOME_CASES)
    {
        rnd sim(0, 0, c.keepGoing, 1);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);

        rnd::CostOutcome outcome = sim.cost();
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
    rnd sim(0, 0, false, 1);

    sim.processRoll(6);   // fails = 1
    sim.processRoll(18);  // successes = 1
    sim.processRoll(18);  // successes = 2
    sim.processRoll(18);  // successes = 3

    rnd::CostOutcome outcome = sim.cost();
    assert(outcome == rnd::CostOutcome::RetrySuccess);
}

static void testReportCostOutcomes()
{
    for (const CostOutcomeCase& c : COST_OUTCOME_CASES)
    {
        rnd sim(0, 0, c.keepGoing, 1);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);
        sim.processRoll(c.rollValue);

        rnd::CostOutcome outcome = reportCost(sim);
        assert(outcome == c.expectedOutcome);
    }
}

static void testRunRollSequenceReturnsTrueWhenAlreadyAtThreshold()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(18);
    sim.processRoll(18);
    sim.processRoll(18);
    assert(sim.getSuccesses() == 3);

    bool complete = runRollSequence(sim);
    assert(complete == true);
}

static void testRunRollSequenceExhaustsSafetyLimit()
{
    rnd sim(0, 0, false, 1);

    // Force 1000 consecutive neutral rolls - never terminal - so
    // runRollSequence's safetyCounter runs out. Real dice can't reliably
    // reproduce this (any roll outside the neutral band would move
    // successes/fails toward a real threshold within a handful of rolls).
    for (int i = 0; i < 1000; ++i)
    {
        sim.forceNextRoll(10);
    }

    bool complete = runRollSequence(sim);
    assert(complete == false);
}

// promptInt/promptBool read from std::cin directly; redirect its buffer
// to a stringstream so their parse/retry/EOF logic is exercised
// deterministically instead of only via CI's one fixed smoke-test input.
namespace
{
class CinRedirect
{
public:
    explicit CinRedirect(const std::string& input)
        : input_(input), original_(std::cin.rdbuf(input_.rdbuf()))
    {
    }

    ~CinRedirect()
    {
        std::cin.rdbuf(original_);
    }

    CinRedirect(const CinRedirect&) = delete;
    CinRedirect& operator=(const CinRedirect&) = delete;

private:
    std::istringstream input_;
    std::streambuf* original_;
};
}  // namespace

static void testPromptIntParsesValidInteger()
{
    CinRedirect redirect("42\n");

    int value = 0;
    promptInt(value, false);

    assert(value == 42);
}

static void testPromptIntRetriesOnInvalidInputThenAcceptsValid()
{
    CinRedirect redirect("abc\n7\n");

    int value = 0;
    promptInt(value, false);

    assert(value == 7);
}

static void testPromptIntRetriesUntilAboveZero()
{
    CinRedirect redirect("-5\n0\n3\n");

    int value = 0;
    promptInt(value, true);

    assert(value == 3);
}

static void testPromptIntEofLeavesValueUntouched()
{
    CinRedirect redirect("");

    int value = 99;
    promptInt(value, false);

    assert(value == 99);
}

static void testPromptBoolParsesYesNoCaseInsensitive()
{
    {
        CinRedirect redirect("Y\n");
        bool result = false;
        promptBool(result);
        assert(result == true);
    }
    {
        CinRedirect redirect("N\n");
        bool result = true;
        promptBool(result);
        assert(result == false);
    }
}

static void testPromptBoolRetriesOnInvalidThenAccepts()
{
    CinRedirect redirect("q\ny\n");

    bool result = false;
    promptBool(result);

    assert(result == true);
}

static void testPromptBoolEofDefaultsFalse()
{
    CinRedirect redirect("");

    bool result = true;
    promptBool(result);

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
    assert(config.numRolls == 0);
    assert(config.keepGoing == false);
}

static void testRunSessionZeroRollsCompletesImmediately()
{
    SimConfig config;

    bool completed = runSession(config);
    assert(completed == true);
}

static void testRunIterationCompletesWithoutHanging()
{
    rnd sim(0, 0, true, 1);

    bool completed = runIteration(sim);

    assert(sim.getSuccesses() >= 0);
    assert(sim.getFails() >= 0);
    (void)completed;
}

static void testRunIterationExhaustsRetryLimit()
{
    rnd sim(0, 0, false, 1);

    // Force 100 consecutive success-with-fault cycles (3 rolls of 14 each -
    // successes==3, faults==1 -> CostOutcome::RetrySuccess -> retry), so
    // runIteration's MAX_RETRIES is exhausted on the 100th cycle.
    for (int i = 0; i < 100; ++i)
    {
        sim.forceNextRoll(14);
        sim.forceNextRoll(14);
        sim.forceNextRoll(14);
    }

    bool completed = runIteration(sim);
    assert(completed == false);
}

// The exhaustion path (testRunIterationExhaustsRetryLimit) is already
// deterministic; the ordinary "fails once, retries, then succeeds" path
// was previously only exercised by real (non-deterministic) dice.
static void testRunIterationSucceedsAfterOneRetry()
{
    rnd sim(0, 0, false, 1);

    sim.forceNextRoll(14);  // attempt 1: RetrySuccess -> retry
    sim.forceNextRoll(14);
    sim.forceNextRoll(14);

    sim.forceNextRoll(18);  // attempt 2: clean Success -> done
    sim.forceNextRoll(18);
    sim.forceNextRoll(18);

    bool completed = runIteration(sim);
    assert(completed == true);
}

int main()
{
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
    testReportCostOutcomes();

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
