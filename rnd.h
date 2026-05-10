#pragma once

#include <random>

class rnd
{
private:
    static constexpr int D20_SIDES{ 20 };

    static constexpr int PERFECT_SUCCESS{ 23 };
    static constexpr int SUCCESS{ 18 };
    static constexpr int SUCCESS_WITH_FAULT{ 14 };
    static constexpr int FAILURE_THRESHOLD{ 7 };

    static constexpr int REQUIRED_SUCCESSES{ 3 };
    static constexpr int REQUIRED_FAILURES{ 3 };

    int prior{};

    int successes{};
    int fails{};
    int faults{};

    int costOffset{};

    long long totalCost{};
    long long grandTotalCost{};

    long long multiplier{ 1 };

    bool keepGoing{};

    std::mt19937 rng;

public:
    rnd(int p, int co, bool kg);

    bool roll();
    void processRoll(int rollValue);

    bool cost();

    void reset();
    void incPrior();

    long long getGrandTotalCost() const;
};