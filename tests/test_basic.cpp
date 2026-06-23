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

int main()
{
    testConstructor();
    testReset();
    testPriorIncrement();
    testLargeConstructorValues();
    testMultipleResets();

    std::cout << "All tests passed.\n";

    return 0;
}
