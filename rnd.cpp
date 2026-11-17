#include "rnd.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>

long long Simulator::safeAdd(long long a, long long b)
{
    assert(a >= 0 && b >= 0);

    if (a > std::numeric_limits<long long>::max() - b)
    {
        return std::numeric_limits<long long>::max();
    }

    return a + b;
}

long long Simulator::safeMultiply(long long a, long long b)
{
    assert(a >= 0 && b >= 0);

    if (a == 0 || b == 0)
        return 0;

    if (a > std::numeric_limits<long long>::max() / b)
        return std::numeric_limits<long long>::max();

    return a * b;
}

Simulator::Simulator(int initialPrior, int initialCostOffset, bool continueOnFailure,
                     unsigned int seed, std::function<int()> rollSource)
    : prior(initialPrior),
      successes(0),
      faults(0),
      criticalFailures(0),
      attemptFinalized(false),
      totalCost(0),
      grandTotalCost(0),
      multiplier(1),
      continueOnFailure(continueOnFailure),
      lastRoll(0),
      rollSource(std::move(rollSource)),
      rng(seed)
{
    prior = std::clamp(prior, PRIOR_MIN, PRIOR_MAX);
    const int costOffset = std::clamp(initialCostOffset, COST_OFFSET_MIN, COST_OFFSET_MAX);

    int exponent = costOffset + 3;

    for (int i = 0; i < exponent; ++i)
    {
        multiplier = safeMultiply(multiplier, 10);
    }
}

bool Simulator::isComplete() const
{
    return successes >= REQUIRED_SUCCESSES || criticalFailures >= REQUIRED_CRITICAL_FAILURES;
}

bool Simulator::canCompleteCleanly() const
{
    return D20_SIDES + prior >= SUCCESS;
}

bool Simulator::roll()
{
    if (isComplete())
        return true;

    int rollValue;

    if (rollSource)
    {
        rollValue = rollSource();
    }
    else
    {
        std::uniform_int_distribution<int> d20(1, D20_SIDES);
        rollValue = d20(rng) + prior;
    }

    processRoll(rollValue);

    return isComplete();
}

void Simulator::processRoll(int rollValue)
{
    if (isComplete())
        throw std::logic_error("Reset the completed attempt before rolling again.");

    std::uniform_int_distribution<int> costRoll(1, D20_SIDES);

    long long cost = costRoll(rng);

    totalCost = safeAdd(totalCost, cost);
    lastRoll = rollValue;

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
        ++criticalFailures;
    }
}

Simulator::CostOutcome Simulator::finalizeAttempt()
{
    if (attemptFinalized)
        return finalizedOutcome;
    if (!isComplete())
        throw std::logic_error("Complete the attempt before finalizing its cost.");

    CostOutcome outcome;
    if (successes >= REQUIRED_SUCCESSES)
    {
        outcome =
            faults > 0 || criticalFailures > 0 ? CostOutcome::RetrySuccess : CostOutcome::Success;
    }
    else
    {
        outcome = continueOnFailure ? CostOutcome::RetryFailure : CostOutcome::GiveUpFailure;
    }

    if (outcome == CostOutcome::Success || outcome == CostOutcome::GiveUpFailure)
    {
        grandTotalCost = safeAdd(grandTotalCost, totalCost);
    }

    finalizedOutcome = outcome;
    attemptFinalized = true;
    return outcome;
}

void Simulator::reset()
{
    successes = 0;
    faults = 0;
    criticalFailures = 0;
    attemptFinalized = false;
    totalCost = 0;
}

void Simulator::incrementPrior()
{
    if (prior < MAX_ESCALATED_PRIOR)
    {
        ++prior;
    }
}

long long Simulator::getGrandTotalCost() const
{
    return safeMultiply(grandTotalCost, multiplier);
}

long long Simulator::getScaledCost() const
{
    return safeMultiply(totalCost, multiplier);
}

int Simulator::getLastRoll() const
{
    return lastRoll;
}

int Simulator::getSuccesses() const
{
    return successes;
}

int Simulator::getFaults() const
{
    return faults;
}

int Simulator::getCriticalFailures() const
{
    return criticalFailures;
}
