# Engine for Breakthrough


<img width="617" height="674" alt="Breakthrough" src="https://github.com/user-attachments/assets/926bddd7-21f6-43b6-9012-4b46884a8107" />

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
- branchless bucket R/W

On the way:
- BUGFIXING
- multi-threading
- SIMD (?)
- Tk debloating
- removal of Numba leftovers
- might revisit associative bucketing

<h2>Empirical results</h2>

I determined experimentally the optimal size for TT for depth 8 to be ~2^21 entries (`8MB` for 32-bit entries, `16MB` for 64-bit ones). I assume it is because it covers most board states searched (around 2M), while fitting in L3 on most x86 CPUs.


<h2>Current entry structure by bits</h2>

`1` - **generation**
Determines if the entry is from the current or previous search.

`2` - **depth**
Representing values 1-8 while ignoring parity, with the replacement condition prioritizing the minimizer on equal depth values. It is possible to delegate only this much because a board state is practically guarranteed to reoccur only at the same depth in a single search and the current player is mostly deducible from it.

`2` - **flag**
3 states for upper/lower bound and exact score.

`5` - **best move index**
The best in the first 32 moves from a deterministically calculated list of possible moves. If it becomes viable, I might merge it with **flag** to increase this to 42 moves, since `3 x 42 = 126` states can be encoded in 7 bits too.

`7` - **score**
Representing values from -63 to +63. The score is usually clamped between -62 and +62 with special hardcodings for pieces that reach the end.

`15` - **hash remainder**
Additional bits to check against to prevent collisions. With 21 bits used for indexing TT, it allows 36 total bits of verification.

<h2>[ OLD ]  Devised 13-way associativity</h2>


<img width="1641" height="994" alt="layout" src="https://github.com/user-attachments/assets/b826d8aa-5242-41ef-bbae-08991c5c8827" />


