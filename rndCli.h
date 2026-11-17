#pragma once

#include "rnd.h"

#include <iostream>
#include <optional>

// Return false when input ends before a valid value is available.
bool promptInt(int& value, bool aboveZero, std::istream& input = std::cin,
               std::ostream& output = std::cout);
bool promptBool(bool& result, std::istream& input = std::cin, std::ostream& output = std::cout);

struct SimConfig
{
    int prior = 0;
    int costOffset = 0;
    int numIterations = 0;
    bool continueOnFailure = false;
};

enum class RunResult
{
    Completed,
    RollLimitExceeded,
    RetryLimitExceeded,
    InvalidConfig
};

std::optional<SimConfig> promptConfig(std::istream& input = std::cin,
                                      std::ostream& output = std::cout);

void printCost(const Simulator& simulator, Simulator::CostOutcome outcome,
               std::ostream& output = std::cout);

RunResult runRollSequence(Simulator& simulator, std::ostream& output = std::cout);
RunResult runIteration(Simulator& simulator, std::ostream& output = std::cout,
                       std::ostream& errors = std::cerr);
RunResult runSession(const SimConfig& config, std::ostream& output = std::cout,
                     std::ostream& errors = std::cerr);
