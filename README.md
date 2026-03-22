# Engine for Breakthrough


<img width="617" height="674" alt="Breakthrough" src="https://github.com/user-attachments/assets/926bddd7-21f6-43b6-9012-4b46884a8107" />

<br>

<br>

<h2>Instructions</h2>

Ran with:


```sh build.sh```



<h2>Requirements</h2>

- clang
- numpy + Tk


<h2>Optimization</h2>

Current best compromise for speed and difficulty is depth 8. The old raw `u64` array approach is still faster. To try the 

The classic Minimax optimizations:
- transposition table (TT)
- best move tracking
- alpha/beta pruning
- Zobrist hashing
- move sorting
- `constexpr` pos value matrix

Additional features implemented so far:
- 32-bit TT entries
- 14-way associative bucketing
- branchless incremental score/hash eval
- branchless moves eval
- branchless bucket R/W

On the way:
- BUGFIXING
- multi-threading
- SIMD (?)
- `__builtin_prefetch`
- serious performance analysis with `perf`
- Tk debloating
- removal of Numba leftovers


<h2>[ OLD ]  Devised 13-way associativity</h2>


<img width="1641" height="994" alt="layout" src="https://github.com/user-attachments/assets/b826d8aa-5242-41ef-bbae-08991c5c8827" />


