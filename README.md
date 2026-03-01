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

Features on the way:
- reducing TT entry from 8 to 4 bytes
- 64-byte TT buckets, using 12-13 / 16 slots for entries and the rest for collision checking


<h2>Memory bit layouts</h2>

<h3>Current:</h3>

- move **[8]**: **3** (row) + **3** (column) + **2** (move type)
- TT entry **[64]**: **16** (best move) + **2** (a/b flag) + **4** (depth) + **18** (score) + **24** (hash remainder)

<h3>Future:</h3>

- move **[16]**:  **3** (row)  +  **3** (column)  +  **2** (move type)  +  **8** (original index, for best move insertion)
- TT entry **[32]**:  **6** (best move index)  +  **2** (a/b flag)  +  **4** (depth)  +  **8** (score)  +  **12** (hash remainder)

<br>

<img width="496" height="196" alt="image" src="https://github.com/user-attachments/assets/5a53f931-9710-4b44-8f4c-e23b03d65e0f" />
