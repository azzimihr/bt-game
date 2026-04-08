# Engine for Breakthrough


<img width="617" height="674" alt="Breakthrough" src="https://github.com/user-attachments/assets/926bddd7-21f6-43b6-9012-4b46884a8107" style="display: block; margin: 0 auto;"/>

<br>

<br>

<h2>Instructions</h2>

Play with:

```./build.sh```

Benchmark with:

```./build.sh bench```


<h2>Requirements</h2>

- clang & libc++
- numpy & Tk
- Linux (for now)

The game currently auto-starts at depth 9, due to it being sufficiently fast.

<h2>Optimization</h2>


The classic Minimax optimizations:
- alpha/beta pruning
- move sorting
- transposition table (TT)
- best move tracking
- Zobrist hashing

Additional features implemented so far:
- atomic TT + lockless multithreading (2.5x speed)
- 32-bit TT entries (1/2x memory)
- incremental score/hash eval with bitboards (avoids recomputation)
- `constexpr` position value matrix

On the way:
- proper thread pool
- dynamic core count detection
- further SIMD utilization
- Negamax
- possibly improved locality by again reorganizing the TT

<h2>32-bit entry structure</h2>

<img width="809" height="300" alt="Figure_2" src="https://github.com/user-attachments/assets/84788a08-4e5d-40bc-8a11-3ebe7f268883" style="display: block; margin: 0 auto;"/>


<br>

<br>

`2` - **generation**

Determines if the entry is from the current search or one of the previous searches.

`2` - **depth**

Representing values 1-8 while ignoring parity, with the replacement condition prioritizing the minimizer on equal depth values. It is possible to delegate only this much because a board state is practically guarranteed to reoccur only at the same depth in a single search.

`2` - **flag**

3 states for upper/lower bound and exact score.

`5` - **best move index**

The best in the first 42 moves from a deterministically calculated list of possible moves. Now merged with **flag** to increase the limit from 32 moves, since `3 x 42 = 126` states can be encoded in 7 bits too.

`7` - **score**

Representing values from -63 to +63. The score is usually clamped between -62 and +62 with special hardcodings for pieces that reach the end.

`14` - **hash remainder**

Additional bits to check against to prevent collisions. With 21 bits used for indexing TT, it allows for a total of 35 bits of verification.

<h2>Multithreading effectiveness</h2>

<img width="475" height="286" alt="image" src="https://github.com/user-attachments/assets/4f4b206f-d224-437d-8134-5b88e9c9fdde" />

Plateaus after exceeding CPU core count. There also seem to be around 350ms of unparallelizable work per round.

<h2>Other insights</h2>

The average branching factor after the root is around 6.

<br>

# FORMER OPTIMIZATION EXPERIMENTS

<h3>[ OUTDATED ] Optimal TT/cache size</h3>

*I have in the meantime found ways to effectively eliminate most of the cache latency, so cache size doesn't matter much anymore. Leaving this here for historical reference.* 

I determined experimentally the optimal size for TT for depth 8 to be ~2^21 entries (`8MB` for 32-bit entries, `16MB` for 64-bit ones). I assume it is because it covers most board states searched (around 2M), while fitting in L3 on most x86 CPUs.

<img width="783" height="502" alt="graph5" src="https://github.com/user-attachments/assets/21354799-02fa-4f52-bab4-8c376beda6c5" style="display: block; margin: 0 auto;"/>

This is plotting the benchmark results involving random moves from the minimizer and calculated moves by the maximizer. The benchmarks were done in separate batches for the first 4, 5, 6, 8 and 10 moves, with the arithmetic mean boldened. As expected, the results stabilize after a certain threshold (~ 2^19 here), and plateau up to ~ 2^22, after which cache misses start to affect the performance.

<img width="657" height="214" alt="image" src="https://github.com/user-attachments/assets/4011647d-754c-40bf-8a1a-7fc2edb94063" style="display: block; margin: 0 auto;"/>

Execution time data from `perf` after collapsing recursion.

<h3>[ OUTDATED ]  Devised 13-way associativity</h3>

*This was an experimental idea on my part to pack entries into buckets which would contain a header with additional hash bits along with a few bits to represent the current permutation of an LRU bucket segment. Turned out to be way too slow to index and so i ditched it. Tried with 12 and 14-way associativity too.*

<img width="1641" height="994" alt="layout" src="https://github.com/user-attachments/assets/b826d8aa-5242-41ef-bbae-08991c5c8827" style="display: block; margin: 0 auto;"/>


