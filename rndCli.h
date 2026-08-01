#pragma once

#include "rnd.h"

void promptInt(int& value, bool aboveZero);
void promptBool(bool& result);

struct SimConfig
{
    int prior;
    int costOffset;
    int numRolls;
    bool keepGoing;
};

SimConfig promptConfig();

bool runRollSequence(rnd& simulator);

// Not a pure report: also drives the cost() state transition
// (finalizes grandTotalCost on Success/GiveUpFailure) as a side effect,
// in addition to printing the outcome.
rnd::CostOutcome reportCost(rnd& simulator);

bool runIteration(rnd& simulator);
bool runSession(const SimConfig& config);
