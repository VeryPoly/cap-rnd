#include "rndCli.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

constexpr bool ABOVE_ZERO = true;
constexpr bool ANY_INTEGER = false;

namespace
{
void warnIfClamped(int value, int min, int max, const char* label, std::ostream& output)
{
    if (value < min || value > max)
    {
        output << label << " is clamped to the range [" << min << ", " << max << "].\n";
    }
}
}  // namespace

bool promptInt(int& value, bool aboveZero, std::istream& input, std::ostream& output)
{
    std::string line;
    while (std::getline(input, line))
    {
        std::istringstream parsed(line);
        int candidate;
        if (parsed >> candidate && (parsed >> std::ws).eof() && (!aboveZero || candidate > 0))
        {
            value = candidate;
            return true;
        }
        output << (aboveZero ? "Enter a number greater than zero: " : "Enter a valid number: ");
    }
    return false;
}

bool promptBool(bool& result, std::istream& input, std::ostream& output)
{
    std::string line;
    while (std::getline(input, line))
    {
        std::istringstream parsed(line);
        char candidate = 0;
        parsed >> candidate >> std::ws;
        candidate = static_cast<char>(std::tolower(static_cast<unsigned char>(candidate)));
        if (!parsed.fail() && parsed.eof() && (candidate == 'y' || candidate == 'n'))
        {
            result = candidate == 'y';
            return true;
        }
        output << "Enter y or n: ";
    }
    result = false;
    return false;
}

std::optional<SimConfig> promptConfig(std::istream& input, std::ostream& output)
{
    SimConfig config;

    output << "Prior: ";
    if (!promptInt(config.prior, ANY_INTEGER, input, output))
        return std::nullopt;
    warnIfClamped(config.prior, Simulator::PRIOR_MIN, Simulator::PRIOR_MAX, "Prior", output);

    output << "Cost Offset: ";
    if (!promptInt(config.costOffset, ANY_INTEGER, input, output))
        return std::nullopt;
    warnIfClamped(config.costOffset, Simulator::COST_OFFSET_MIN, Simulator::COST_OFFSET_MAX,
                  "Cost Offset", output);

    output << "Number of Iterations: ";
    if (!promptInt(config.numIterations, ABOVE_ZERO, input, output))
        return std::nullopt;

    output << "Keep Going if failed? (y/n): ";
    if (!promptBool(config.continueOnFailure, input, output))
        return std::nullopt;

    return config;
}

RunResult runRollSequence(Simulator& simulator, std::ostream& output)
{
    if (simulator.isComplete())
        return RunResult::Completed;

    constexpr int MAX_ROLLS_PER_ATTEMPT = 1000;
    int remainingRolls = MAX_ROLLS_PER_ATTEMPT;

    for (;;)
    {
        bool complete = simulator.roll();

        output << simulator.getLastRoll() << ' ';

        if (complete)
            return RunResult::Completed;

        if (--remainingRolls <= 0)
            return RunResult::RollLimitExceeded;
    }
}

void printCost(const Simulator& simulator, Simulator::CostOutcome outcome, std::ostream& output)
{
    bool succeeded = outcome == Simulator::CostOutcome::RetrySuccess ||
                     outcome == Simulator::CostOutcome::Success;

    if (succeeded)
    {
        output << "\nFaults: " << simulator.getFaults() << ", " << simulator.getCriticalFailures()
               << " critical failures\n";
    }

    output << "Total cost: " << simulator.getScaledCost();

    switch (outcome)
    {
        case Simulator::CostOutcome::RetrySuccess:
            output << " success, redoing to remove fault.\n\n";
            break;

        case Simulator::CostOutcome::Success:
            output << " success.\n\n";
            break;

        case Simulator::CostOutcome::RetryFailure:
        case Simulator::CostOutcome::GiveUpFailure:
            output << " failed.\n\n";
            break;
    }
}

RunResult runIteration(Simulator& simulator, std::ostream& output, std::ostream& errors)
{
    constexpr int MAX_RETRIES = 100;
    for (int attempt = 0; attempt <= MAX_RETRIES; ++attempt)
    {
        const auto result = runRollSequence(simulator, output);
        if (result != RunResult::Completed)
        {
            errors << "Safety limit exceeded.\n";
            return result;
        }

        const auto outcome = simulator.finalizeAttempt();
        printCost(simulator, outcome, output);
        simulator.reset();

        if (outcome == Simulator::CostOutcome::Success ||
            outcome == Simulator::CostOutcome::GiveUpFailure)
        {
            simulator.incrementPrior();
            return RunResult::Completed;
        }
    }

    errors << "Retry limit exceeded.\n";
    return RunResult::RetryLimitExceeded;
}

RunResult runSession(const SimConfig& config, std::ostream& output, std::ostream& errors)
{
    Simulator simulator(config.prior, config.costOffset, config.continueOnFailure);

    if (config.numIterations > 0 && config.continueOnFailure && !simulator.canCompleteCleanly())
    {
        errors << "Prior must be at least -2 when continuing after failures.\n";
        return RunResult::InvalidConfig;
    }

    int completed = 0;

    while (completed < config.numIterations)
    {
        const auto result = runIteration(simulator, output, errors);
        if (result != RunResult::Completed)
            return result;

        ++completed;
    }

    output << "Grand total cost across all iterations: " << simulator.getGrandTotalCost() << "\n\n";

    return RunResult::Completed;
}
