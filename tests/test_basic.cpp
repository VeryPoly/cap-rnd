#include "rnd.h"
#include <cassert>

int main()
{
    rnd sim(0, 0, false);
    // initial grand total should be zero
    assert(sim.getGrandTotalCost() == 0);
    return 0;
}
