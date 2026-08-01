cap-rnd

A lightweight C++ simulator for CAP-style rolling systems.

The program simulates repeated d20 rolls, tracks successes, faults, and critical failures, and calculates cumulative costs across completed iterations.

---

Features

- Native C++ implementation
- Lightweight executable
- No external runtime dependencies
- Cross-platform support
- Configurable roll modifiers
- Configurable cost scaling
- Automatic cost tracking
- Deterministic simulation rules

---

Supported Platforms

- Windows
- Linux
- macOS

---

Requirements

Running Releases

No additional software is required.

Building From Source

- C++17 compatible compiler or newer

Supported compilers:

- GCC 11+
- Clang 14+
- Visual Studio 2022+

---

Installation

Download the latest release from the [repository's Releases page](https://github.com/VeryPoly/cap-rnd/releases/latest) and extract the archive.

No installation process is required.

---

Windows

Usage

1. Download:

[cap-rnd-win64.zip](https://github.com/VeryPoly/cap-rnd/releases/latest/download/cap-rnd-win64.zip)

2. Extract the archive.

3. Run:

rnd.exe

---

Linux

Usage

1. Download:

[cap-rnd-linux-x64.tar.gz](https://github.com/VeryPoly/cap-rnd/releases/latest/download/cap-rnd-linux-x64.tar.gz)

2. Extract:

tar -xzf cap-rnd-linux-x64.tar.gz

3. Open a terminal in the extracted directory.

4. Make the binary executable:

chmod +x rnd

5. Run:

./rnd

---

macOS

Usage

1. Download:

[cap-rnd-macos.tar.gz](https://github.com/VeryPoly/cap-rnd/releases/latest/download/cap-rnd-macos.tar.gz)

2. Extract:

tar -xzf cap-rnd-macos.tar.gz

3. Open Terminal inside the extracted directory.

4. Run:

./rnd

---

Building From Source

GCC

g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic rnd.cpp rndCli.cpp rndMain.cpp -o rnd

Clang

clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic rnd.cpp rndCli.cpp rndMain.cpp -o rnd

MSVC

cl /std:c++17 /O2 rnd.cpp rndCli.cpp rndMain.cpp

---

Program Usage

After launching the application, the following inputs are requested:

Input| Description
Prior| Modifier applied to every d20 roll
Cost Offset| Cost scaling exponent
Number of Rolls| Number of completed simulations
Keep Going| Continue after failed attempts

Example:

Prior: 2
Cost Offset: 1
Number of Rolls: 5
Keep Going if failed? (y/n): y

---

Mechanics

Roll Resolution

Every roll uses:

d20 + Prior

The modified result determines the outcome.

Result| Outcome
23+| Perfect Success
18–22| Success
14–17| Success with Fault
7–13| Neutral Result
6 or less| Critical Failure

---

Success Conditions

An attempt ends when either:

- 3 successes are accumulated
- 3 critical failures are accumulated

A Perfect Success immediately satisfies the success requirement.

---

Faults

Faults represent imperfect successes.

Depending on simulator settings and roll outcomes, attempts containing faults may be retried.

---

Critical Failures

Critical failures contribute toward the failure threshold.

Accumulating three critical failures causes the attempt to fail.

---

Cost Calculation

Each roll generates a cost value.

The costs generated during an attempt are accumulated and multiplied by a scaling factor derived from the selected Cost Offset.

Multipliers

Cost Offset| Multiplier
-3| 1
-2| 10
-1| 100
0| 1,000
1| 10,000
2| 100,000
3| 1,000,000
4| 10,000,000
5| 100,000,000
6| 1,000,000,000

---

Example Output

Prior: 2
Cost Offset: 1
Number of Rolls: 1
Keep Going if failed? (y/n): y

18 22 14

Faults: 1, 0 crit faults
Total cost: 470000 success.

Grand total cost across all iterations: 470000

---

Project Structure

.
├── rnd.h
├── rnd.cpp
├── rndCli.h
├── rndCli.cpp
├── rndMain.cpp
├── tests/
│   └── test_basic.cpp
├── scripts/
│   └── format.sh
├── .github/
│   ├── workflows/
│   └── release-drafter.yml
├── .clang-format
├── .pre-commit-config.yaml
├── LICENSE
└── README.md

---

Development

Install pre-commit hooks:

pip install pre-commit
pre-commit install

Format source code:

./scripts/format.sh

Run tests:

g++ -std=c++17 tests/test_basic.cpp rndCli.cpp rnd.cpp -o test_runner
./test_runner

---

License

Licensed under the MIT License.

Copyright (c) 2026 Poly4324
Copyright (c) 2026 Camellia

See the LICENSE file for the full license text.
