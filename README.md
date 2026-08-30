# lxySort

[![CI](https://github.com/MEMZ-CHROER/Lxy-Sort/actions/workflows/ci.yml/badge.svg)](https://github.com/MEMZ-CHROER/Lxy-Sort/actions)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![header-only](https://img.shields.io/badge/header--only-yes-brightgreen.svg)]()
[![tests](https://img.shields.io/badge/stress-6606%20passed-brightgreen.svg)]()

**A header-only, adaptive, high-performance sorting library for C++17.**

`lxy_sort.hpp` is a single include-only header with **zero dependencies** (just the C++ standard library). It automatically picks the fastest algorithm for your data — and beats `std::sort` in the large majority of cases.

> 中文版说明见 [README_zh.md](./README_zh.md)

---

## Quick start

```cpp
#include "lxy_sort.hpp"

std::vector<int> a = {5,3,9,1,4,3,7,2,8,6};
lxySort(a);                            // stable, auto-picks the fastest algorithm
lxySortUnstable(a);                    // max-speed mode (stable not required)
lxySort(a, std::greater<int>{});       // custom comparator
lxySortByKey(recs, [](const Rec& r){ return r.id; });                     // stable by single key
lxySortByKey(recs, [](const Rec& r){ return std::make_tuple(r.id, r.name); }); // stable by multiple keys
lxySortParallel(a, cmp);               // parallel (comparison-based, needs -fopenmp)
```

Build (Windows/MinGW or Linux, no third-party dependency):

```
g++ -O2 -std=c++17 your.cpp -o your              # serial
g++ -O2 -std=c++17 -fopenmp your.cpp -o your     # enable parallel
```

---

## Supported types

- **All integers**: `int` / `unsigned` / `long` / `long long` / `short` / `char` ... (signed & unsigned)
- **Floating point**: `float` / `double` (negatives, `-0.0`, subnormals, ±infinity handled)
- **Strings**: `std::string` (MSD radix sort)
- **Any comparable type**: user structs (just needs `operator<`, like `std::sort`)

---

## Algorithm dispatch

After a single O(n) detection scan, it dispatches to the best algorithm:

| Data shape | Algorithm | Complexity |
|-----------|-----------|-----------|
| Already sorted / all same | early return | O(n) |
| Strictly reversed | reverse | O(n) |
| Tiny arrays | insertion / merge | constant |
| Small range (range ≤ n) | counting sort | O(n) |
| Nearly sorted | natural merge | O(n·log runs) |
| Large arrays, wide range (int/float) | radix sort | O(n) |
| Few distinct full-range (e.g. only min/max) | 2-value fill | O(n) |
| Strings | MSD radix | O(n·L) |
| Generic structs / custom comparator | stable merge | O(n log n) |

Average/common cases run in **O(n)**; worst case **O(n log n)**; extra space **O(n)** (reusable thread-local workspace — no per-call allocation).

---

## Benchmark vs `std::sort` (N=1M, g++ -O2)

| Scenario | Algorithm | Speedup |
|----------|-----------|---------|
| Ascending / All same | O(n) early exit | 12–18x |
| Descending | O(n) reverse | 9x |
| Rotated sorted | counting | 11x |
| Random 0..999 | counting | 5.5x |
| Random 0..1e9 | radix | 3.9–6x |
| 2 extremes (only INT_MIN/INT_MAX) | 2-value fill | 2.6x |
| double Ascending | O(n) | 16.9x |
| Random string | MSD radix | 3.3x |
| uint64 Random | 64-bit radix | 2.7x |
| float Random wide | radix | 7.4x |
| stable random keys | half-in-place merge | ~1.0x (ties) |

**61/64** benchmark scenarios beat `std::sort` (full coverage of 64 scenarios across int/uint64/int64/float/double/string/stable/byKey/parallel/dispatch boundaries). 6606 stress tests pass; zero warnings with `-Wall`.

---

## Full complexity breakdown (all bench scenarios)

Dispatch: sorted/reversed `O(n)` → small-range counting/bitmap `O(n)` → nearly-sorted natural merge `O(n·log runs)` → wide-range radix `O(n)` → string MSD radix `O(n·L)` → generic comparison `O(n log n)`.

### int

| Scenario | Shape | Algorithm | Time | Space |
|---|---|---|---|---|
| Random 0..1e9 | wide-range random | radix | **O(n)** | O(n) |
| Random 0..999999 | wide-range random | radix | **O(n)** | O(n) |
| Random 0..999 | small range | counting | **O(n)** | O(n) |
| Random 0..99 | small range | counting | **O(n)** | O(n) |
| Duplicates 0..9 | small range | counting | **O(n)** | O(n) |
| Negative -100k..100k | wide range | radix | **O(n)** | O(n) |
| Mixed signs wide | wide range | radix | **O(n)** | O(n) |
| Ascending | sorted | early exit | **O(n)** | O(1) |
| Ascending + repeats | sorted | early exit | **O(n)** | O(1) |
| Descending | reversed | reverse | **O(n)** | O(1) |
| Descending + repeats | reversed | reverse | **O(n)** | O(1) |
| All same | all same | early exit | **O(n)** | O(1) |
| Nearly sorted (10 swaps) | range=n unique | bitmap | **O(n)** | O(n/8) |
| Nearly sorted (200 swaps) | range=n unique | bitmap | **O(n)** | O(n/8) |
| Nearly sorted wide-range | near-sorted | natural merge | **O(n·log runs)** | O(n) |
| Snake pattern | range=n unique | bitmap | **O(n)** | O(n/8) |
| Zigzag peak-valley | small range | counting | **O(n)** | O(n) |
| Rotated sorted | range=n unique | bitmap | **O(n)** | O(n/8) |
| Few distinct (0..2) | small range | counting | **O(n)** | O(n) |
| Random permutation 1..n | range=n unique | bitmap | **O(n)** | O(n/8) |
| 90% sorted + random tail | wide range | radix | **O(n)** | O(n) |
| Half sorted + random | wide range | radix | **O(n)** | O(n) |
| Descending (comparator) | reversed | reverse | **O(n)** | O(1) |
| int32 full range | wide range | radix | **O(n)** | O(n) |
| **2 extremes (max dup)** | only min/max | 2-value fill | **O(n)** | O(1) |
| Organ-pipe | small range | counting | **O(n)** | O(n) |
| Exponential dist | small range | counting | **O(n)** | O(n) |
| INT_MIN/MAX/neg mix | wide range | radix | **O(n)** | O(n) |
| Bit-reversal 0..2^20 | wide range | radix | **O(n)** | O(n) |

### uint64 / int64 (64-bit radix)

| Scenario | Shape | Algorithm | Time | Space |
|---|---|---|---|---|
| uint64 Random | wide range | 64-bit radix | **O(8n)=O(n)** | O(n) |
| uint64 Ascending | sorted | early exit | **O(n)** | O(1) |
| uint64 Descending | reversed | reverse | **O(n)** | O(1) |
| int64 Random | wide range | 64-bit radix | **O(8n)=O(n)** | O(n) |
| int64 Mixed signs | wide range | 64-bit radix | **O(8n)=O(n)** | O(n) |

### double

| Scenario | Shape | Algorithm | Time | Space |
|---|---|---|---|---|
| double Random wide | wide range | radix | **O(n)** | O(n) |
| double Random 0..999 | FP small | radix | **O(n)** | O(n) |
| double Ascending | sorted | early exit | **O(n)** | O(1) |
| double Descending | reversed | reverse | **O(n)** | O(1) |
| double Nearly sorted | near-sorted | natural merge | **O(n·log runs)** | O(n) |
| double All same | all same | early exit | **O(n)** | O(1) |
| double Mixed signs | wide range | radix | **O(n)** | O(n) |

### float

| Scenario | Shape | Algorithm | Time | Space |
|---|---|---|---|---|
| float Random wide | wide range | radix | **O(n)** | O(n) |
| float Random 0..999 | FP small | radix | **O(n)** | O(n) |
| float Ascending | sorted | early exit | **O(n)** | O(1) |
| float Descending | reversed | reverse | **O(n)** | O(1) |
| float Mixed signs | wide range | radix | **O(n)** | O(n) |
| float All same | all same | early exit | **O(n)** | O(1) |

### string

| Scenario | Shape | Algorithm | Time | Space |
|---|---|---|---|---|
| string Random | variable-length | MSD radix | **O(n·L)** | O(n) |
| string Ascending | sorted | early exit | **O(n)** | O(1) |
| string Descending | reversed | reverse | **O(n)** | O(1) |
| string Nearly sorted | near-sorted | natural merge | **O(n·log runs)** | O(n) |
| string Few distinct | variable-length | MSD radix | **O(n·L)** | O(n) |

### stable / byKey

| Scenario | Shape | Algorithm | Time | Space |
|---|---|---|---|---|
| stable many-dup keys | many duplicates | half-in-place merge | **O(n log n)** | O(n) |
| stable near-sorted keys | near-sorted | natural merge | **O(n·log runs)** | O(n) |
| stable random keys | random | half-in-place merge | **O(n log n)** | O(n) |
| byKey int | key int | radix by key | **O(n)** | O(n) |
| byKey double | key double | radix by key | **O(n)** | O(n) |

### Different sizes (Random 0..999999)

| N | Algorithm | Time | Space |
|---|---|---|---|
| N=16 | insertion sort | **O(n²)** worst / const | O(1) |
| N=17 | introsort | **O(n log n)** avg | O(log n) |
| N=100 | introsort | **O(n log n)** avg | O(log n) |
| N=128 | introsort | **O(n log n)** avg | O(log n) |
| N=129 | introsort | **O(n log n)** avg | O(log n) |
| N=1000 | radix | **O(n)** | O(n) |
| N=10000 | radix | **O(n)** | O(n) |
| N=100000 | radix | **O(n)** | O(n) |
| N=1000000 | counting | **O(n)** | O(n) |

### Parallel

| Scenario | Algorithm | Time | Space |
|---|---|---|---|
| Struct sort(by a) N=1M | block sort + merge | **O(n log n / P)** | O(n) |
| Struct sort(by a) N=3M | block sort + merge | **O(n log n / P)** | O(n) |

**Bottom line vs `std::sort`:** every scenario whose data can be linearized (integers, floats, small ranges, strings) runs in **O(n)**; only truly comparison-only generic types fall back to **O(n log n)** (on par with `std`). This is the complexity source of the 4–7x (radix), 8–11x (counting/bitmap), and 15x (ascending) wins.

---

## Honest limitations

1. **Stable sort of generic types** (e.g. random `std::string`) is slower than `std::sort`'s *unstable* introsort — this is the intrinsic cost of stability (same as `std::stable_sort`). Use `lxySortUnstable` for speed.
2. **Micro arrays (N≤128)**: roughly ties `std::sort` (sub-microsecond noise — `libstdc++`'s introsort is itself extremely optimized; our introsort is a faithful reimplementation and wins on average but is within ±10% measurement noise at this scale).
3. **Parallel only helps comparison-heavy sorts**: radix/counting/string-radix are memory-bandwidth bound (no gain). `lxySortParallel` mainly helps CPU-bound comparison types (struct ~2–3x).
4. **Requires O(n) extra memory** (not in-place); the array is reordered.
5. Linear sorts (radix/counting) engage only when the comparator is the default `std::less` and `T` is arithmetic.

---

## Files

- `lxy_sort.hpp` — the core implementation (the only file you need to include)
- `example.cpp` — minimal usage example
- `stress.cpp` / `stress2.cpp` — correctness + stability tests (incl. parallel / multi-key)
- `bench.cpp` — 64-scenario benchmark vs `std::sort`
- `run_tests.sh` / `run_tests.bat` — one-click test scripts

## Run the tests

```
./run_tests.sh            # or Windows: run_tests.bat
./run_tests.sh bench      # additionally run the performance benchmark
./bench.exe 1000000       # or run the benchmark directly, N is configurable
```

---

## License

[MIT](./LICENSE) © 2025 MEMZ-CHROER
