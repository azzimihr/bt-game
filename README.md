# Engine for Breakthrough


<img width="617" height="674" alt="Breakthrough" src="https://github.com/user-attachments/assets/926bddd7-21f6-43b6-9012-4b46884a8107" style="display: block; margin: 0 auto;"/>

<br>

<br>

<h2>Instructions</h2>

> The game currently auto-starts at depth 9, due to it being sufficiently fast. You can manually 

Play with:
&nbsp;
```./build.sh```



Benchmark with:
&nbsp;
```./build.sh bench```


<h2>Requirements</h2>


- clang, libc++
- Python 3.x, numpy, Tk
- Linux, perf (for now)
- Hotspot (for benchmarking)

<h2>Optimization</h2>


The classic Minimax optimizations:
- alpha/beta pruning
- move sorting
- transposition table (TT)
- best move tracking
- Zobrist hashing

Additional features implemented so far:
- atomic TT + lockless multithreading (~2.8x speed)
- 32-bit TT entries (1/2x memory)
- incremental score/hash eval with bitboards (avoids recomputation)
- `constexpr` position value matrix

On the way:
- proper thread pool
- dynamic core count detection
- further SIMD utilization
- possibly improved locality by again reorganizing the TT

<h2>32-bit entry structure</h2>

<img width="886" height="346" alt="rgb" src="https://github.com/user-attachments/assets/f3ff8e9d-b8d1-4619-9bc5-ba0851fbf853" />


<br>

<br>

`2` - **depth**

> Representing values 1-8 while ignoring parity, with the replacement condition prioritizing the minimizer on equal depth values. It is possible to delegate only this much because a board state is practically guarranteed to reoccur only at the same depth in a single search.

`5.42` - **best move index**

> *Shares bits with **flag**.*
The best in the first 42 moves from a deterministically calculated list of possible moves.

`1.58` - **flag**

> *Shares bits with **best move index**.*
3 states for upper/lower/exact bound.

`7.42` - **score**

> *Shares bits with **generation**.*
Representing values from -84 to +84. The score is usually clamped between -83 and +83 with special hardcodings for pieces that reach the end.

`1.58` - **generation**

> *Shares bits with **score**.*
Determines if the entry is from the current search or one of the previous searches.

`14` - **hash remainder**

> Additional bits to check against to prevent collisions. With 20-27 bits used for indexing TT, it allows for a total of 34-41 bits of verification.

<h2>Insights</h2>

<h3>Multithreading effectiveness</h2>

<img width="475" height="286" alt="image" src="https://github.com/user-attachments/assets/4f4b206f-d224-437d-8134-5b88e9c9fdde" />

Plateaus after exceeding CPU core count. There also seem to be around 350ms of unparallelizable work per round. Effective thread utilization is ~87% which could be improved

<h3>Other</h2>

The average branching factor after the root is around 30, but due to pruning, increasing depth by 1 effectively increases time by a factor of 4-6.

<img width="814" height="211" alt="Screenshot_20260424_165931" src="https://github.com/user-attachments/assets/e1837403-6056-408c-abf9-907ef435f105" />

Execution time data from `perf` after collapsing recursion. This is plotting the benchmark results involving random moves from the minimizer and calculated moves by the maximizer.

## FORMER OPTIMIZATION EXPERIMENTS

<h3>[ OUTDATED ] Optimal TT/cache size</h3>

> *I have in the meantime found ways to effectively eliminate most of the cache latency, so cache size doesn't matter much anymore. Leaving it here for historical reference.* 

I determined experimentally the optimal size for TT for depth 8 to be ~2^21 entries (`8MB` for 32-bit entries, `16MB` for 64-bit ones). I assume it is because it covers most board states searched (around 2M), while fitting in L3 on most x86 CPUs.

<img width="783" height="502" alt="graph5" src="https://github.com/user-attachments/assets/21354799-02fa-4f52-bab4-8c376beda6c5" style="display: block; margin: 0 auto;"/>

The benchmarks were done in separate batches for the first 4, 5, 6, 8 and 10 moves, with the arithmetic mean boldened. As expected, the results stabilize after a certain threshold (~ 2^19 here), and plateau up to ~ 2^22, after which cache misses start to affect the performance.

<h3>[ OUTDATED ]  Devised 13-way associativity</h3>

> *This was an experimental idea on my part to pack entries into buckets which would contain a header with additional hash bits along with a few bits to represent the current permutation of an LRU bucket segment. Turned out to be way too slow to index and so i ditched it. Tried with 12 and 14-way associativity too.*

<img width="1641" height="994" alt="layout" src="https://github.com/user-attachments/assets/b826d8aa-5242-41ef-bbae-08991c5c8827" style="display: block; margin: 0 auto;"/>


