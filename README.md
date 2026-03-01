# Engine for Breakthrough


<img width="617" height="674" alt="Breakthrough" src="https://github.com/user-attachments/assets/926bddd7-21f6-43b6-9012-4b46884a8107" />

<br>

<br>

<h2>Requirements</h2>

- Python 3.x
- NumPy

- GCC / Numba


<h2>Optimization</h2>

Currently manages Depth **8** on my Ryzen 5500U.

Has all the textbook optimizations of the **Minimax algorithm**:
- transposition table (in raw array form)
- best move tracking
- alpha/beta pruning
- Zobrist hashing
- collision handling
- move sorting

Additional features implemented so far:
- static NumPy arrays and matrices for all data
- bit-packing of TT entries and moves
- precalculated position weight matrix
- incremental score/hash calculation
- parallel bitboard representation for branchless move calculation
- C++ rewrite

On the way:
- reducing TT entry from 8 to 4 bytes
- 64-byte TT buckets, using 13 / 16 slots for entries and the rest for collision checking
- depth- vs LRU-based bucket segments
- removal of Numba
- Tkinter debloating
- bugfixing


<h2>Devised memory layout (WIP)</h2>


<img width="1641" height="994" alt="layout" src="https://github.com/user-attachments/assets/b826d8aa-5242-41ef-bbae-08991c5c8827" />


