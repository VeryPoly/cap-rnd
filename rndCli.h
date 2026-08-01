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
rnd::CostOutcome reportCost(rnd& simulator);
bool runIteration(rnd& simulator);
bool runSession(const SimConfig& config);
