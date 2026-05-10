#include "rnd.h"

#include <iostream>

rnd::rnd(int p, int co, bool kg)
    : prior(p),
      costOffset(co),
      keepGoing(kg),
      rng(std::random_device{}())
{
    for (int i = 0; i < costOffset + 3; ++i)
    {
        multiplier *= 10;
    }
}

void rnd::reset()
{
    successes = 0;
    fails = 0;
    faults = 0;

    totalCost = 0;
}

bool rnd::roll()
{
    std::uniform_int_distribution<int> d20(1, D20_SIDES);

    int rollValue = d20(rng) + prior;

    processRoll(rollValue);

    std::cout << rollValue << ", ";

    return successes >= REQUIRED_SUCCESSES
        || fails >= REQUIRED_FAILURES;
}

void rnd::processRoll(int rollValue)
{
    std::uniform_int_distribution<int> costRoll(1, D20_SIDES);

    long long cost = costRoll(rng);

    if (rollValue >= PERFECT_SUCCESS)
    {
        successes = REQUIRED_SUCCESSES;

        totalCost += cost;
        grandTotalCost += cost;
    }
    else if (rollValue >= SUCCESS)
    {
        ++successes;

        totalCost += cost;
        grandTotalCost += cost;
    }
    else if (rollValue >= SUCCESS_WITH_FAULT)
    {
        ++successes;
        ++faults;

        totalCost += cost;
        grandTotalCost += cost;
    }
    else if (rollValue >= FAILURE_THRESHOLD)
    {
        totalCost += cost;
        grandTotalCost += cost;
    }
    else
    {
        ++fails;

        totalCost += cost;
        grandTotalCost += cost;
    }
}

bool rnd::cost()
{
    if (fails < REQUIRED_FAILURES)
    {
        std::cout
            << "Faults: "
            << faults
            << ", "
            << fails
            << " crit faults\n";
    }

    long long scaledCost = totalCost * multiplier;

    std::cout << "Total cost: " << scaledCost;

    if (successes >= REQUIRED_SUCCESSES)
    {
        if (faults >= 1 || fails >= 1)
        {
            std::cout << " success, redoing to remove fault.\n\n";
            return true;
        }

        std::cout << " success.\n\n";
        return false;
    }

    std::cout << " failed.\n\n";

    return keepGoing;
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
    return grandTotalCost * multiplier;
}