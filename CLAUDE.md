# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A lightweight C++20 CLI simulator for CAP-style rolling systems. Simulates repeated d20 rolls, tracks successes/faults/critical failures per attempt, and accumulates cost across completed iterations. No external runtime dependencies.

## Build

```
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic rnd.cpp rndCli.cpp rndMain.cpp -o rnd
```

(Clang: swap `g++` for `clang++`. MSVC: `cl /std:c++20 /O2 rnd.cpp rndCli.cpp rndMain.cpp`.)

## Test

```
g++ -std=c++20 tests/test_basic.cpp rndCli.cpp rnd.cpp -o test_runner
./test_runner
```

There's a single test binary (`tests/test_basic.cpp`) with plain `assert`-based checks, no test framework. Add new cases as additional `static void testX()` functions and call them from `main()`. There is no way to run a single test in isolation — the whole binary always runs all of them.

CI (`.github/workflows/ci.yml`) also runs a smoke test against the built `rnd` binary by piping fixed stdin: `printf "0\n0\n1\nn\nn\n" | ./rnd`.

## Format

Formatting is enforced via `clang-format` (config in `.clang-format`) and checked in CI with `--dry-run --Werror`. Format before committing:

```
./scripts/format.sh
```

This runs `clang-format -i` over every `.cpp`/`.h` file tracked by git. Pre-commit hooks (`.pre-commit-config.yaml`) also run clang-format plus basic hygiene checks (trailing whitespace, EOF fixer, etc.) — install with `pre-commit install`.

## Architecture

Three translation units, split cleanly between simulation logic, CLI orchestration, and the process entry point:

- **`rnd.h` / `rnd.cpp`** — the `rnd` class, all simulation state and rules. Does **no I/O at all** — `roll()` and `cost()` return data (a bool and a `CostOutcome` enum) instead of printing, and expose `getLastRoll()`/`getScaledCost()` so callers can print. Construction clamps `prior` to [-20, 20] and `costOffset` to [-3, 6], and precomputes the cost `multiplier` (10^(costOffset+3)) at construction time.
- **`rndCli.h` / `rndCli.cpp`** — CLI helpers and orchestration: `promptInt`/`promptBool` (retry-on-invalid-input loops, take references not pointers), `promptConfig()`, and the simulation-driving functions `runRollSequence()`/`reportCost()`/`runIteration()`/`runSession()`. Owns all the printing. Split out from `rndMain.cpp` specifically so these functions are linkable into `tests/test_basic.cpp` without pulling in `main()` (which would collide at link time).
- **`rndMain.cpp`** — just `main()`: the outer "run another simulation?" loop, calling `promptConfig()`/`runSession()` from `rndCli.h`.

Core simulation flow per attempt (see `rndCli.cpp`'s `runIteration()`):
1. `runRollSequence()` loops calling `roll()` until it returns true (3 successes or 3 critical failures reached), bounded by a safety counter (1000).
2. `reportCost()` calls `cost()` to score the attempt, gets back a `CostOutcome` (`RetrySuccess`/`Success`/`RetryFailure`/`GiveUpFailure`), and prints the corresponding message.
3. `reset()` clears per-attempt state (successes/faults/fails/totalCost) between retries; `incPrior()` bumps `prior` by 1 (capped at the named constant `MAX_ESCALATED_PRIOR = 7`) after each *completed* iteration (not each retry).
4. `getGrandTotalCost()` accumulates the scaled cost of only the iterations counted as final (success or give-up-without-keepGoing).

Roll outcome thresholds and success/failure requirements are `static constexpr` in `rnd.h` — change them there, not as magic numbers elsewhere. `safeAdd`/`safeMultiply` are saturating arithmetic helpers guarding against `long long` overflow on cost accumulation; use them for any new arithmetic on cost or totals rather than raw `+`/`*`. `safeMultiply` asserts both operands are non-negative — every current call site already guarantees this, so if you ever call it elsewhere, keep that true or the assert will fire.

Test coverage: `tests/test_basic.cpp` covers `rnd`'s engine logic thoroughly (boundary-exact on every roll threshold) via a seeded constructor for determinism, plus `runRollSequence`/`reportCost`/`runSession` from `rndCli.h` where the outcome can be made deterministic (pre-loading engine state via `processRoll()` rather than relying on the real RNG). `runIteration()` can only get a smoke test (no crash/hang) since it always calls the real RNG at least once via its `do`/`while` structure.
