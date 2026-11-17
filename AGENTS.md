# Project memory

This file provides repository context for Codex and other coding agents. Read CLAUDE.md for additional build guidance and docs/DEVELOPER_GUIDE.md for the architecture and API. Keep these files aligned when changing behavior or public names.

## Project and architecture

- The GitHub repository is VeryPoly/cap-rnd. This is a small C++20 command-line simulator with three translation units and no external runtime dependencies.
- rnd.h and rnd.cpp contain Simulator, which owns simulation state and performs no console I/O. The original type name rnd remains a compatibility alias.
- rndCli.h and rndCli.cpp own input, output, and session orchestration. Functions accept streams so tests can use local string streams.
- rndMain.cpp contains the application entry point. Tests link rnd.cpp and rndCli.cpp with their own main function.
- tests/test_basic.cpp uses plain assertions, seeded engines, and a test-owned ScriptedSimulator fixture. An injected roll source supplies already-modified scores. Cost rolls still use the seeded RNG.

## Behavior to preserve

- A roll generates d20 + prior and a separate cost between 1 and 20. Scores of 23 or higher immediately complete the success requirement. Other thresholds are defined in rnd.h.
- An attempt ends at three successes or three critical failures. Successful attempts containing any fault or critical failure retry. Failed attempts retry when continueOnFailure is enabled.
- Only Success and GiveUpFailure add attempt cost to the session total. Costs from retried attempts are discarded. Cost getters apply the multiplier derived from the cost offset.
- finalizeAttempt() is idempotent until reset(). Finalizing an incomplete attempt or directly processing another roll after completion throws std::logic_error.
- roll() on a completed attempt returns true without changing state. runRollSequence() on a completed attempt produces no further output.
- reset() clears attempt counters, cost, and finalization status. Session total, prior, RNG state, and last-roll value persist.
- Each attempt permits at most 1000 rolls. Each iteration permits one initial attempt plus 100 retries. Increment prior only after a completed iteration, capped at 7. Initial priors above 7 remain unchanged.
- Positive sessions with continueOnFailure enabled require a clamped prior of at least -2, since lower priors cannot produce a clean success.
- safeAdd() and safeMultiply() accept nonnegative operands and saturate at the maximum long long value.
- Input helpers validate entire lines and report EOF. Integer destinations change only after validation. Boolean EOF sets false. An incomplete configuration yields an empty optional, and main exits without starting a session.
- Orchestration returns RunResult so callers can distinguish completion, roll-limit exhaustion, retry-limit exhaustion, and invalid configuration.

## Naming and code conventions

- Use PascalCase for types, camelCase for functions and variables, and UPPER_SNAKE_CASE for constants.
- Current names include criticalFailures, getCriticalFailures(), continueOnFailure, attemptFinalized, incrementPrior(), and numIterations.
- Use the term critical failures in output and documentation. numIterations counts completed iterations.
- Keep finalization separate from printing. Use finalizeAttempt() and printCost(). The old cost() and reportCost() wrappers were removed.
- Keep state transitions explicit and preserve the assertions in focused tests when consolidating setup.

## Validation

Build the application from the repository root with GCC or Clang.

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic rnd.cpp rndCli.cpp rndMain.cpp -o rnd
```

Build and run the tests with assertions enabled.

```sh
g++ -std=c++20 -O2 -UNDEBUG -Wall -Wextra -Wpedantic tests/test_basic.cpp rndCli.cpp rnd.cpp -o test_runner
./test_runner
printf '0\n0\n1\nn\nn\n' | ./rnd
```

- Formatting uses clang-format 22.1.8 and the repository's C++20 Allman configuration. scripts/format.sh operates on tracked C++ files.
- CI also runs pre-commit, cppcheck, sanitizers, coverage, and platform builds. Shared build flags live in .github/actions/build-and-test/action.yml.
- Evaluate functions before asserting their results. Cppcheck rejects potentially mutating calls inside assert(), and those calls disappear when assertions are disabled.
- Confirm actual logs and results before reporting a check as passed. Local test success does not establish that a release workflow was executed.

## GitHub and privacy

- The user requested that personal email references be removed. Keep personal email addresses out of repository files, commit messages, and commit metadata. Use the configured GitHub noreply identity and verify both author and committer.
- The GitHub plugin and local Git authenticate separately. Local Git was connected through Git Credential Manager's browser flow. Keep credentials in the credential manager.
- Git and compilers may be missing from PATH on this Windows machine. Locate available tools before concluding they are unavailable. Temporary validation tools used during this session were removed after testing.

## Handoff recorded on 2026-11-17

- PR #31 covers duplicated test setup and consolidated release artifact downloads on refactor/remove-duplicated-test-and-release-code. Recheck its current state before updating it.
- Later cleanup, simplification, naming, and consistency changes were validated locally. Inspect git status and the diff to determine the current commit state before staging or switching branches.
- Tests, warning-clean application and test builds, clang-format, and CLI checks passed for the latest local changes. The tag-triggered release workflow was not exercised end to end.
- macOS builds were moved from unsupported macos-14 to macos-15. A previous run remained queued during inspection. Recheck GitHub for its current outcome.
- Release Drafter conditions now use when.labels and explicit version-resolver categories. Platform artifact downloads use an anchored name pattern with merge_multiple disabled to preserve separate packaging directories.
