#include "../rnd.h"

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

    assert(sim.roll() == true);
}

static void testRollReturnsTrueOnceFailThresholdReached()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(6);
    sim.processRoll(6);
    sim.processRoll(6);
    assert(sim.getFails() == 3);

    assert(sim.roll() == true);
}

static void testCostSuccessWithoutFaultFinalizes()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(18);
    sim.processRoll(18);
    sim.processRoll(18);

    assert(sim.cost() == false);
    assert(sim.getGrandTotalCost() > 0);
}

static void testCostSuccessWithFaultRetries()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(14);
    sim.processRoll(14);
    sim.processRoll(14);

    assert(sim.cost() == true);
    assert(sim.getGrandTotalCost() == 0);
}

static void testCostFailureKeepGoingRetries()
{
    rnd sim(0, 0, true, 1);

    sim.processRoll(6);
    sim.processRoll(6);
    sim.processRoll(6);

    assert(sim.cost() == true);
    assert(sim.getGrandTotalCost() == 0);
}

static void testCostFailureNoKeepGoingFinalizes()
{
    rnd sim(0, 0, false, 1);

    sim.processRoll(6);
    sim.processRoll(6);
    sim.processRoll(6);

    assert(sim.cost() == false);
    assert(sim.getGrandTotalCost() > 0);
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

    testCostSuccessWithoutFaultFinalizes();
    testCostSuccessWithFaultRetries();
    testCostFailureKeepGoingRetries();
    testCostFailureNoKeepGoingFinalizes();

    std::cout << "All tests passed.\n";

    return 0;
}
