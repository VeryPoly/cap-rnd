# cap-rnd

A lightweight RNG-based rolling simulator written in C++.

---

# Features

- Portable console application
- Cross-platform support
- No installation required
- Lightweight native executable
- Simple terminal interface

---

# Supported Platforms

- Windows
- Linux

---

# Installation

No installation is required.

Simply download the correct release for your operating system from the Releases page and extract the archive.

---

# Windows

## Usage

1. Download:

```text
cap-rnd-win64.zip
```

2. Extract the ZIP archive.

3. Run:

```text
rnd.exe
```

---

# Linux

## Usage

1. Download:

```text
cap-rnd-linux-x64.tar.gz
```

2. Extract:

```bash
tar -xzf cap-rnd-linux-x64.tar.gz
```

3. Open a terminal in the extracted folder.

4. Make the binary executable:

```bash
chmod +x rnd
```

5. Run:

```bash
./rnd
```
---

# Program Usage

After launching the program, you will be prompted for:

| Input | Description |
|---|---|
| Prior | Roll modifier |
| Cost Offset | Multiplier scaling |
| Number of Rolls | Number of iterations |
| Keep Going | Retry failed attempts |

---

# Example

```text
Prior: 2
Cost Offset: 1
Number of Rolls: 5
Keep Going if failed? (y/n): y
```

---

# License

Licensed under the MIT License.
