#pragma once

#include <limits>
#include <queue>
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

    // Escalation cap for incPrior(), distinct from the constructor's
    // wider [-20, 20] clamp on the initial prior.
    static constexpr int MAX_ESCALATED_PRIOR = 7;

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

    // Test-only override queue: roll() consumes from here first if
    // non-empty, falling back to the real RNG otherwise. Empty by
    // default, so production behavior is unaffected.
    std::queue<int> forcedRolls;

    std::mt19937 rng;

    static long long safeAdd(long long a, long long b);
    static long long safeMultiply(long long a, long long b);

public:
    // Bounds the constructor silently clamps prior/costOffset to.
    static constexpr int PRIOR_MIN = -20;
    static constexpr int PRIOR_MAX = 20;
    static constexpr int COST_OFFSET_MIN = -3;
    static constexpr int COST_OFFSET_MAX = 6;

    enum class CostOutcome
    {
        RetrySuccess,  // succeeded, but a fault or fail occurred - attempt retries
        Success,       // succeeded with no fault or fail - attempt finalizes
        RetryFailure,  // failed (3 crit fails), keepGoing is true - attempt retries
        GiveUpFailure  // failed (3 crit fails), keepGoing is false - attempt finalizes
    };

    rnd(int p, int co, bool kg, unsigned int seed = std::random_device{}());

    bool roll();
    void processRoll(int rollValue);

    CostOutcome cost();

    void reset();
    void incPrior();

    // Queues an explicit value for the next roll() call to use instead
    // of drawing from the RNG, for deterministic testing.
    void forceNextRoll(int rollValue);

    // Cumulative cost across every finalized iteration this session.
    long long getGrandTotalCost() const;

    // Cost of the current (not-yet-finalized) attempt only.
    long long getScaledCost() const;

    int getLastRoll() const;

    int getSuccesses() const;
    int getFaults() const;
    int getFails() const;
};
