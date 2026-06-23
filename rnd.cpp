#include "rnd.h"

#include <iostream>

long long rnd::safeAdd(long long a, long long b)
{
    if (b > 0 &&
        a > std::numeric_limits<long long>::max() - b)
    {
        return std::numeric_limits<long long>::max();
    }

    if (b < 0 &&
        a < std::numeric_limits<long long>::min() - b)
    {
        return std::numeric_limits<long long>::min();
    }

    return a + b;
}

long long rnd::safeMultiply(long long a, long long b)
{
    if (a == 0 || b == 0)
        return 0;

    if (a > std::numeric_limits<long long>::max() / b)
        return std::numeric_limits<long long>::max();

    return a * b;
}

rnd::rnd(int p, int co, bool kg)
    : prior(p),
      costOffset(co),
      successes(0),
      faults(0),
      fails(0),
      totalCost(0),
      grandTotalCost(0),
      multiplier(1),
      keepGoing(kg),
      rng(std::random_device{}())
{
    if (prior < -20)
        prior = -20;

    if (prior > 20)
        prior = 20;

    if (costOffset < -3)
        costOffset = -3;

    if (costOffset > 6)
        costOffset = 6;

    int exponent = costOffset + 3;

    for (int i = 0; i < exponent; ++i)
    {
        multiplier = safeMultiply(multiplier, 10);
    }
}

bool rnd::roll()
{
    std::uniform_int_distribution<int> d20(1, D20_SIDES);

    int rollValue = d20(rng) + prior;

    processRoll(rollValue);

    std::cout << rollValue << ' ';

    return successes >= REQUIRED_SUCCESSES
        || fails >= REQUIRED_FAILURES;
}

void rnd::processRoll(int rollValue)
{
    std::uniform_int_distribution<int> costRoll(1, D20_SIDES);

    long long cost = costRoll(rng);

    totalCost = safeAdd(totalCost, cost);

    if (rollValue >= PERFECT_SUCCESS)
    {
        successes = REQUIRED_SUCCESSES;
    }
    else if (rollValue >= SUCCESS)
    {
        ++successes;
    }
    else if (rollValue >= SUCCESS_WITH_FAULT)
    {
        ++successes;
        ++faults;
    }
    else if (rollValue >= FAILURE_THRESHOLD)
    {
        // Neutral result
    }
    else
    {
        ++fails;
    }
}

bool rnd::cost()
{
    bool success =
        successes >= REQUIRED_SUCCESSES;

    bool retry =
        success &&
        (faults > 0 || fails > 0);

    long long scaledCost =
        safeMultiply(totalCost, multiplier);

    if (fails < REQUIRED_FAILURES)
    {
        std::cout
            << "\nFaults: "
            << faults
            << ", "
            << fails
            << " crit faults\n";
    }

    std::cout
        << "Total cost: "
        << scaledCost;

    if (retry)
    {
        std::cout
            << " success, redoing to remove fault.\n\n";

        return true;
    }

    if (success)
    {
        grandTotalCost =
            safeAdd(grandTotalCost, totalCost);

        std::cout
            << " success.\n\n";

        return false;
    }

    std::cout
        << " failed.\n\n";

    if (keepGoing)
    {
        return true;
    }

    grandTotalCost =
        safeAdd(grandTotalCost, totalCost);

    return false;
}

void rnd::reset()
{
    successes = 0;
    faults = 0;
    fails = 0;
    totalCost = 0;
}

void rnd::incPrior()
{
    if (prior < 7)
    {
        ++prior;
    }
}

long long rnd::getGrandTotalCost() const
{
    return safeMultiply(grandTotalCost, multiplier);
}

int rnd::getSuccesses() const
{
    return successes;
}

int rnd::getFaults() const
{
    return faults;
}

int rnd::getFails() const
{
    return fails;
}
