# COA Assignment 2: Q2 Cache Mapping Simulation (C++17)

Simulation of a **4-block cache** that sends the same memory block trace
through the three mapping techniques:

```text
Memory requests: 0, 4, 8, 0, 4, 12
```

| Technique | Layout | Set rule |
|---|---|---|
| Direct-Mapped | 4 sets x 1 way | `set = block mod 4` |
| 2-Way Set-Associative | 2 sets x 2 ways | `set = block mod 2` |
| Fully Associative | 1 set x 4 ways | `set = block mod 1` (any line) |

All three techniques use one engine. A cache is `(blocks, ways)` and the set
count is `blocks / ways`. Replacement inside a set is LRU.

## Result for the given trace

```text
Technique                 Sets x Ways  Hits  Misses  Hit rate  Miss rate  Evictions
1) Direct-Mapped          4 x 1        0     6       0.00%     100.00%    5
2) 2-Way Set-Associative  2 x 2        0     6       0.00%     100.00%    4
3) Fully Associative      1 x 4        2     4       33.33%    66.67%     0
```

Every block in the trace is a multiple of 4, so every request maps to set 0.
The direct-mapped cache and the 2-way cache thrash in that one set. The fully
associative cache holds all four blocks, so the repeats of 0 and 4 are hits.

## Build and run

```bash
g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o cache_sim
./cache_sim                          # the question's example trace
./cache_sim 4 0 4 0                  # smaller trace, 2-way beats direct
./cache_sim 8 0 1 2 3 0 1 2 3        # 8-block cache, custom trace
./cache_sim 8 -a 4 0 1 0 1           # 8-block cache, 4-way
./cache_sim --help
```

`output.txt` is the captured run for the default trace. `docs/terminal_log.txt`
holds the default run and the three extra verification runs.

## Files

```text
main.cpp                                   # simulation (362 lines)
output.txt                                 # default run
docs/terminal_log.txt                      # default and extra runs
make_figure.py                             # Figure 1 in the report
screenshots/                               # figure and terminal captures
krmu_logo.png                              # report front page
Aditya_Shibu_2401201047_COA_Assignment2.md # report source
Aditya_Shibu_2401201047_COA_Assignment2.pdf# report (19 pages)
```

## Report

The report follows the Assignment 1 layout: question, design, results,
analysis, terminal evidence, full code listing, explanation, marking scheme
and reproduction steps.

`Aditya_Shibu_2401201047_COA_Assignment2.pdf`: 19 pages, 8 images.

The built PDF is committed next to the source.

## Source hash

```text
0083c9a1f05e11e459052ca036d5ad18441f12471c8878afa24dc7485dfa856c  main.cpp
```

The hash in the report must match this value.
