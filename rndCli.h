#pragma once

#include "rnd.h"

void promptInt(int& value, bool aboveZero);
void promptBool(bool& result);

struct SimConfig
{
    // Defaults matter, not just documentation: promptInt() only assigns
    // its target on a successful read, so if stdin hits EOF before any
    // input is given, these fields would otherwise be left indeterminate.
    int prior = 0;
    int costOffset = 0;
    int numRolls = 0;
    bool keepGoing = false;
};

SimConfig promptConfig();

bool runRollSequence(rnd& simulator);

// Not a pure report: also drives the cost() state transition
// (finalizes grandTotalCost on Success/GiveUpFailure) as a side effect,
// in addition to printing the outcome.
rnd::CostOutcome reportCost(rnd& simulator);

bool runIteration(rnd& simulator);
bool runSession(const SimConfig& config);
