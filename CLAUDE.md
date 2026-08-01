# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A lightweight C++17 CLI simulator for CAP-style rolling systems. Simulates repeated d20 rolls, tracks successes/faults/critical failures per attempt, and accumulates cost across completed iterations. No external runtime dependencies.

## Build

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic rnd.cpp rndMain.cpp -o rnd
```

(Clang: swap `g++` for `clang++`. MSVC: `cl /std:c++17 /O2 rnd.cpp rndMain.cpp`.)

## Test

```
g++ -std=c++17 tests/test_basic.cpp rnd.cpp -o test_runner
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

Two translation units, split cleanly between simulation logic and I/O:

- **`rnd.h` / `rnd.cpp`** — the `rnd` class, all simulation state and rules. No I/O except the roll value being echoed to stdout inside `roll()`. Construction clamps `prior` to [-20, 20] and `costOffset` to [-3, 6], and precomputes the cost `multiplier` (10^(costOffset+3)) at construction time.
- **`rndMain.cpp`** — interactive CLI: prompts for input (`promptInt`/`promptBool` with retry-on-invalid-input loops), drives the simulation loop, prints results.

Core simulation flow per attempt (see `rndMain.cpp` `main()`):
1. Loop calling `roll()` until it returns true (3 successes or 3 critical failures reached), bounded by a safety counter.
2. Call `cost()` to score the attempt — it prints the outcome and returns whether the attempt should retry (a success containing a fault or fail is retried to "remove the fault", up to `MAX_RETRIES`).
3. `reset()` clears per-attempt state (successes/faults/fails/totalCost) between retries; `incPrior()` bumps `prior` by 1 (capped at 7) after each *completed* iteration (not each retry).
4. `getGrandTotalCost()` accumulates the scaled cost of only the iterations counted as final (success or give-up-without-keepGoing).

Roll outcome thresholds and success/failure requirements are `static constexpr` in `rnd.h` — change them there, not as magic numbers elsewhere. `safeAdd`/`safeMultiply` are saturating arithmetic helpers guarding against `long long` overflow on cost accumulation; use them for any new arithmetic on cost or totals rather than raw `+`/`*`.
