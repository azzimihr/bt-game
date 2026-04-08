
#include "packing.h"

struct PyMove {
    u8 r1, c1, r2, c2;
};

struct alignas(4) Move {
    i8 mt, i, r, c;
};

alignas(64) u8 b[8][8];
alignas(64) thread_local u8 bt[8][8];
alignas(64) thread_local Move alm[9][48];
alignas(64) thread_local i16 scores[64];

constexpr u64 tt3 = 26;
// alignas(64) u32 TT3[1 << tt3] = {0};
auto* TT3 = new atomic<u32>[1<<tt3];
u64 gen = 1;
short minscore = 0;
short maxscore = 0;

alignas(64) u64 zobra[3][8][8] = {};

u64 zh() {
    u64 h = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            h ^= zobra[b[i][j]][i][j];
        }
    }
    return h;
}

alignas(64) constexpr static u8 replica[8][8] = {
    {99, 99, 99, 99, 99, 99, 99, 99},
    {12, 10, 11, 12, 12, 11, 10, 12},
    { 9,  7,  8,  9,  9,  8,  7,  9},
    { 7,  5,  6,  7,  7,  6,  5,  7},
    { 6,  4,  5,  6,  6,  5,  4,  6},
    { 5,  3,  4,  5,  5,  4,  3,  5},
    { 4,  2,  3,  4,  4,  3,  2,  4},
    { 4,  2,  3,  4,  4,  3,  2,  3}
};

i16 temp_diff(u8 r, u8 c, u64 p1, u64 p2, u8 val) {
    i64 pos = r * 8 + c;
    u64 pattern = (c == 0) ? 0b1100000000000000110ULL:
                  (c == 7) ? 0b0110000000000000011ULL:
                             0b1110000000000000111ULL;
    u64 mask;
    if (pos>9)
        mask = (pattern << (pos-9));
    else
        mask = (pattern >> (9-pos));

    if (val==1){
        return __builtin_popcountll(p1 & mask) - replica[r][c];
    } else {
        return replica[7-r][c] - __builtin_popcountll(p2 & mask);
    }
}

// i16 temp_diff(u8 r, u8 c, u64 p1, u64 p2, u8 val) {
//     // tie(p1, p2) = bb(b);
//     i8 s = (val==1) ? 1 : -1;
//     int idx = (val==1 ? 0 : 7) + s * r;
//     u64 self = (val == 1) ? p1 : p2;
//     i64 pos = r * 8 + c;
    
//     // Select pattern based on column, strip left/right bits for boundaries
//     u64 pattern = (c == 0) ? 0b1100000000000000110ULL:
//                   (c == 7) ? 0b0110000000000000011ULL:
//                              0b1110000000000000111ULL;
//     u64 mask1 = pattern << (pos-9);
//     u64 mask2 = pattern >> (9-pos);
    
//     int match_count = __builtin_popcountll(self & (pos>9 ? mask1 : mask2));
//     i16 h = match_count - replica[idx][c];
//     return h * s;
// }

i16 move_diff() {
    i16 num = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            u8 v = b[i][j];
            if (v == 1) {
                if (j > 0 && b[i-1][j-1] != 1)
                    num--;
                if (j < 7 && b[i-1][j+1] != 1)
                    num--;
                if (b[i-1][j] == 0)
                    num--;
            } else if (v == 2) {
                if (j > 0 && b[i+1][j-1] != 2)
                    num++;
                if (j < 7 && b[i+1][j+1] != 2)
                    num++;
                if (b[i+1][j] == 0)
                    num++;
            }
        }
    }
    return num;
}

i16 quickeval() {
    i16 h = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            i16 val = b[i][j];
            h += ((4-val)%3-1) * replica[(8 - 1) * (val - 1) - (2 * val - 3) * i][j];
        }
    }
    return h;
}

i8 gameover() {
    for (int i = 0; i < 8; ++i) {
        if (b[0][i] == 1) return -63;
        if (b[7][i] == 2) return 63;
    }
    return 0;
}
i16 state() {
    auto go = gameover();
    if (go) return (go);
    return move_diff() + quickeval();
}

void sorter2(u8 turn, u8 d, u8 bm, i64 tm, i16 score0, u64 p10, u64 p20) {
    u8 total = tm;
    i8 delta = 2 * turn - 1;
    
    for (int i = 0; i < total; ++i) {
        Move m = alm[d][i];
        auto r2 = m.r + delta;
        auto c2 = m.c + m.mt;
        u8 pos = 8*m.r + m.c;
        u64 p2 = p20 & ~(1ull << (pos)) | (1ull << (8*r2 + c2));
        u64 p1 = p10 & ~(1ull << (8*r2 + c2));
        u8 old = bt[r2][c2];
        i16 score = score0 - temp_diff(m.r, m.c, p10, p20, turn+1);
        if (old != 0) {
            score -= temp_diff(r2, c2, p10, p20, old);
        }
        bt[m.r][m.c] = 0;
        bt[r2][c2] = turn + 1;
        score = score + temp_diff(r2, c2, p1, p2, turn+1);
        scores[i] = score;
        bt[m.r][m.c] = turn + 1;
        bt[r2][c2] = old;
    }
    
    for (int i = bm; i < total; ++i) {
        auto key_s = scores[i];
        auto key_m = alm[d][i];
        int j = i - 1;
        while (j >= bm && (turn ? scores[j] < key_s : scores[j] > key_s)) {
            scores[j + 1] = scores[j];
            alm[d][j + 1] = alm[d][j];
            j--;
        }
        scores[j + 1] = key_s;
        alm[d][j + 1] = key_m;
    }
}
