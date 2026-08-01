#include "rnd.h"

#include <cassert>

long long rnd::safeAdd(long long a, long long b)
{
    if (b > 0 && a > std::numeric_limits<long long>::max() - b)
    {
        return std::numeric_limits<long long>::max();
    }

    if (b < 0 && a < std::numeric_limits<long long>::min() - b)
    {
        return std::numeric_limits<long long>::min();
    }

    return a + b;
}

long long rnd::safeMultiply(long long a, long long b)
{
    assert(a >= 0 && b >= 0);

    if (a == 0 || b == 0)
        return 0;

    if (a > std::numeric_limits<long long>::max() / b)
        return std::numeric_limits<long long>::max();

    return a * b;
}

rnd::rnd(int p, int co, bool kg, unsigned int seed)
    : prior(p),
      costOffset(co),
      successes(0),
      faults(0),
      fails(0),
      totalCost(0),
      grandTotalCost(0),
      multiplier(1),
      keepGoing(kg),
      lastRoll(0),
      rng(seed)
{
    if (prior < PRIOR_MIN)
        prior = PRIOR_MIN;

    if (prior > PRIOR_MAX)
        prior = PRIOR_MAX;

    if (costOffset < COST_OFFSET_MIN)
        costOffset = COST_OFFSET_MIN;

    if (costOffset > COST_OFFSET_MAX)
        costOffset = COST_OFFSET_MAX;

    int exponent = costOffset + 3;

    for (int i = 0; i < exponent; ++i)
    {
        multiplier = safeMultiply(multiplier, 10);
    }
}

bool rnd::roll()
{
    int rollValue;

    if (!forcedRolls.empty())
    {
        rollValue = forcedRolls.front();
        forcedRolls.pop();
    }
    else
    {
        std::uniform_int_distribution<int> d20(1, D20_SIDES);
        rollValue = d20(rng) + prior;
    }

    processRoll(rollValue);

    lastRoll = rollValue;

    return successes >= REQUIRED_SUCCESSES || fails >= REQUIRED_FAILURES;
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

rnd::CostOutcome rnd::cost()
{
    bool success = successes >= REQUIRED_SUCCESSES;
    bool retry = success && (faults > 0 || fails > 0);

    CostOutcome outcome;

    if (retry)
    {
        outcome = CostOutcome::RetrySuccess;
    }
    else if (success)
    {
        outcome = CostOutcome::Success;
    }
    else if (keepGoing)
    {
        outcome = CostOutcome::RetryFailure;
    }
    else
    {
        outcome = CostOutcome::GiveUpFailure;
    }

    if (outcome == CostOutcome::Success || outcome == CostOutcome::GiveUpFailure)
    {
        grandTotalCost = safeAdd(grandTotalCost, totalCost);
    }

    return outcome;
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
    if (prior < MAX_ESCALATED_PRIOR)
    {
        ++prior;
    }
}

void rnd::forceNextRoll(int rollValue)
{
    forcedRolls.push(rollValue);
}

long long rnd::getGrandTotalCost() const
{
    return safeMultiply(grandTotalCost, multiplier);
}

long long rnd::getScaledCost() const
{
    return safeMultiply(totalCost, multiplier);
}

int rnd::getLastRoll() const
{
    return lastRoll;
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
