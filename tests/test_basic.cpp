#include "../rnd.h"
#include "../rndCli.h"

#include <cassert>
#include <iostream>

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

static void testLargeConstructorValues()
{
    rnd simPositive(1000000, 1000000, false);
    rnd simNegative(-1000000, -1000000, false);

    assert(simPositive.getGrandTotalCost() == 0);
    assert(simNegative.getGrandTotalCost() == 0);
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

static void testProcessRollPerfectSuccess()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(23);

    assert(sim.getSuccesses() == 3);
    assert(sim.getFaults() == 0);
    assert(sim.getFails() == 0);
}

static void testProcessRollSuccessBoundaries()
{
    rnd simLow(0, 0, false, 1);
    simLow.processRoll(18);
    assert(simLow.getSuccesses() == 1);
    assert(simLow.getFaults() == 0);

    rnd simHigh(0, 0, false, 1);
    simHigh.processRoll(22);
    assert(simHigh.getSuccesses() == 1);
    assert(simHigh.getFaults() == 0);
}

static void testProcessRollSuccessWithFaultBoundaries()
{
    rnd simLow(0, 0, false, 1);
    simLow.processRoll(14);
    assert(simLow.getSuccesses() == 1);
    assert(simLow.getFaults() == 1);

    rnd simHigh(0, 0, false, 1);
    simHigh.processRoll(17);
    assert(simHigh.getSuccesses() == 1);
    assert(simHigh.getFaults() == 1);
}

static void testProcessRollNeutralBoundaries()
{
    rnd simLow(0, 0, false, 1);
    simLow.processRoll(7);
    assert(simLow.getSuccesses() == 0);
    assert(simLow.getFaults() == 0);
    assert(simLow.getFails() == 0);

    rnd simHigh(0, 0, false, 1);
    simHigh.processRoll(13);
    assert(simHigh.getSuccesses() == 0);
    assert(simHigh.getFaults() == 0);
    assert(simHigh.getFails() == 0);
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

int main()
{
    testConstructor();
    testReset();
    testPriorIncrement();
    testLargeConstructorValues();
    testMultipleResets();

    testProcessRollPerfectSuccess();
    testProcessRollSuccessBoundaries();
    testProcessRollSuccessWithFaultBoundaries();
    testProcessRollNeutralBoundaries();
    testProcessRollCriticalFailure();
    testProcessRollAccumulatesToThreshold();

    testRollReturnsTrueOnceSuccessThresholdReached();
    testRollReturnsTrueOnceFailThresholdReached();

    testCostOutcomes();
    testReportCostOutcomes();

    testRunRollSequenceReturnsTrueWhenAlreadyAtThreshold();
    testRunRollSequenceExhaustsSafetyLimit();

    testSimConfigDefaultValues();
    testRunSessionZeroRollsCompletesImmediately();
    testRunIterationCompletesWithoutHanging();
    testRunIterationExhaustsRetryLimit();

    std::cout << "All tests passed.\n";

    return 0;
}
