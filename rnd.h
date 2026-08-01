#pragma once

#include <limits>
#include <random>

class rnd
{
private:
    static constexpr int D20_SIDES = 20;

    static constexpr int PERFECT_SUCCESS = 23;
    static constexpr int SUCCESS = 18;
    static constexpr int SUCCESS_WITH_FAULT = 14;
    static constexpr int FAILURE_THRESHOLD = 7;

    static constexpr int REQUIRED_SUCCESSES = 3;
    static constexpr int REQUIRED_FAILURES = 3;

    int prior;
    int costOffset;

    int successes;
    int faults;
    int fails;

    long long totalCost;
    long long grandTotalCost;

    long long multiplier;

    bool keepGoing;

    int lastRoll;

    std::mt19937 rng;

    static long long safeAdd(long long a, long long b);
    static long long safeMultiply(long long a, long long b);

public:
    enum class CostOutcome
    {
        RetrySuccess,
        Success,
        RetryFailure,
        GiveUpFailure
    };

    rnd(int p, int co, bool kg, unsigned int seed = std::random_device{}());

    bool roll();
    void processRoll(int rollValue);

    CostOutcome cost();

    void reset();
    void incPrior();

    long long getGrandTotalCost() const;
    long long getScaledCost() const;

    int getLastRoll() const;

    int getSuccesses() const;
    int getFaults() const;
    int getFails() const;
};
