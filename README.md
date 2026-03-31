# Engine for Breakthrough


<img width="617" height="674" alt="Breakthrough" src="https://github.com/user-attachments/assets/926bddd7-21f6-43b6-9012-4b46884a8107" style="display: block; margin: 0 auto;"/>

<br>

<br>

<h2>Instructions</h2>

Ran with:


```./build.sh```



<h2>Requirements</h2>

- clang
- numpy + Tk


<h2>Optimization</h2>

Current best compromise for speed and difficulty is depth 8. The old raw array approach is still faster than bucketing.

The classic Minimax optimizations:
- transposition table (TT)
- best move tracking
- alpha/beta pruning
- Zobrist hashing
- move sorting
- `constexpr` pos value matrix

Additional features implemented so far:
- 32-bit TT entries
- branchless incremental score/hash eval

On the way:
- BUGFIXING
- multi-threading
- SIMD (?)
- Tk debloating
- Negamax
- removal of Numba leftovers
- might revisit associative bucketing

<h2>32-bit entry structure</h2>

<img width="809" height="300" alt="Figure_2" src="https://github.com/user-attachments/assets/84788a08-4e5d-40bc-8a11-3ebe7f268883" style="display: block; margin: 0 auto;"/>

*

<br>

<br>

`2` - **generation**

Determines if the entry is from the current search or one of the previous searches.

`2` - **depth**

Representing values 1-8 while ignoring parity, with the replacement condition prioritizing the minimizer on equal depth values. It is possible to delegate only this much because a board state is practically guarranteed to reoccur only at the same depth in a single search.

`2` - **flag**

3 states for upper/lower bound and exact score.

`5` - **best move index**

The best in the first 32 moves from a deterministically calculated list of possible moves. If it becomes viable, I might merge it with **flag** to increase this to 42 moves, since `3 x 42 = 126` states can be encoded in 7 bits too.

`7` - **score**

Representing values from -63 to +63. The score is usually clamped between -62 and +62 with special hardcodings for pieces that reach the end.

`14` - **hash remainder**

Additional bits to check against to prevent collisions. With 21 bits used for indexing TT, it allows for a total of 35 bits of verification.

<h2>Empirical results</h2>

I determined experimentally the optimal size for TT for depth 8 to be ~2^21 entries (`8MB` for 32-bit entries, `16MB` for 64-bit ones). I assume it is because it covers most board states searched (around 2M), while fitting in L3 on most x86 CPUs.

<img width="783" height="502" alt="graph5" src="https://github.com/user-attachments/assets/21354799-02fa-4f52-bab4-8c376beda6c5" style="display: block; margin: 0 auto;"/>

This is plotting the benchmark results involving random moves from the minimizer and calculated moves by the maximizer. The benchmarks were done in separate batches for the first 4, 5, 6, 8 and 10 moves, with the arithmetic mean boldened. As expected, the results stabilize after a certain threshold (~ 2^19 here), and plateau up to ~ 2^22, after which cache misses start to affect the performance.

<img width="657" height="214" alt="image" src="https://github.com/user-attachments/assets/4011647d-754c-40bf-8a1a-7fc2edb94063" style="display: block; margin: 0 auto;"/>

Execution time data from `perf` after collapsing recursion.

<h2>[ OLD ]  Devised 13-way associativity</h2>


<img width="1641" height="994" alt="layout" src="https://github.com/user-attachments/assets/b826d8aa-5242-41ef-bbae-08991c5c8827" style="display: block; margin: 0 auto;"/>


