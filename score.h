
#include "packing.h"

struct PyMove {
    u8 r1, c1, r2, c2;
};

struct alignas(4) Move {
    i8 mt, i, r, c;
};

alignas(64) u8 b[8][8];
alignas(64) u32 allm[16][64];
alignas(64) Move alm[9][48];
alignas(64) i16 scores[64];
constexpr u64 tt2 = 11;
alignas(64) u64 tt[1 << tt2] = {0};
constexpr u64 tt3 = 21;
alignas(64) u32 TT3[1 << tt3] = {0};
short minscore = 0;
short maxscore = 0;

alignas(64) u64 zobra[3][8][8] = {};
// alignas(64) constexpr u8 replica[8][8] = {
//     {99, 99, 99, 99, 99, 99, 99, 99},
//     {10,  9, 10, 10, 10, 10,  9, 10},
//     { 8,  7,  8,  8,  8,  8,  7,  8},
//     { 6,  5,  6,  6,  6,  6,  5,  6},
//     { 5,  4,  5,  5,  5,  5,  4,  5},
//     { 4,  3,  4,  4,  4,  4,  3,  4},
//     { 3,  2,  3,  3,  3,  3,  2,  3},
//     { 3,  2,  3,  3,  3,  3,  2,  3}
// };

alignas(64) constexpr u8 replica[8][8] = {
    {99, 99, 99, 99, 99, 99, 99, 99},
    {12, 11, 12, 12, 12, 12, 11, 12},
    { 9,  8,  9,  9,  9,  9,  8,  9},
    { 7,  6,  7,  7,  7,  7,  6,  7},
    { 5,  4,  5,  5,  5,  5,  4,  5},
    { 4,  3,  4,  4,  4,  4,  3,  4},
    { 3,  2,  3,  3,  3,  3,  2,  3},
    { 3,  2,  3,  3,  3,  3,  2,  3}
};

u64 gen = 1;


i16 temp_diff(u8 r, u8 c, u64 p1, u64 p2) {
    u8 val = b[r][c];
    // tie(p1, p2) = bb(b);
    i8 s = 3 - 2 * val;
    int idx = 7 * (val - 1) + s * r;
    
    u64 self = (val == 1) ? p1 : p2;
    i64 pos = r * 8 + c;
    
    // Select pattern based on column, strip left/right bits for boundaries
    u64 pattern = (c == 0) ? 0b1100000000000000110ULL:
                  (c == 7) ? 0b0110000000000000011ULL:
                             0b1110000000000000111ULL;
    u64 moveby = abs(pos - 9);
    u64 mask1 = pattern << moveby;
    u64 mask2 = pattern >> moveby;
    
    int match_count = __builtin_popcountll(self & (pos>9 ? mask1 : mask2));
    i16 h = match_count - replica[idx][c];
    return h * s;
}


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

i16 move_sum() {
    i16 num = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            u8 v = b[i][j];
            if (v == 1) {
                if (j > 0 && b[i-1][j-1] != 1)
                    num++;
                if (j < 7 && b[i-1][j+1] != 1)
                    num++;
                if (b[i-1][j] == 0)
                    num++;
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


void sorter(u8 turn, u8 d, u8 bm) {
    u8 total = allm[d][48];
    i8 delta = 2 * turn - 1;
    
    for (int i = 0; i < total; ++i) {
        u8 r, c;
        i8 t;
        tie(r, c, t) = unpack(allm[d][i]);
        u8 old = b[r + delta][c + t];
        b[r][c] = 0;
        b[r + delta][c + t] = turn + 1;
        scores[i] = quickeval();
        b[r][c] = turn + 1;
        b[r + delta][c + t] = old;
    }
    
    for (int i = bm; i < total; ++i) {
        i16 key_s = scores[i];
        u8 key_m = allm[d][i];
        int j = i - 1;
        while (j >= bm && (turn ? scores[j] < key_s : scores[j] > key_s)) {
            scores[j + 1] = scores[j];
            allm[d][j + 1] = allm[d][j];
            j--;
        }
        scores[j + 1] = key_s;
        allm[d][j + 1] = key_m;
    }
}
void sorter2(u8 turn, u8 d, u8 bm, i64 tm) {
    u8 total = tm;
    i8 delta = 2 * turn - 1;
    
    for (int i = 0; i < total; ++i) {
        Move m = alm[d][i];
        u8 old = b[m.r + delta][m.c + m.mt];
        b[m.r][m.c] = 0;
        b[m.r + delta][m.c + m.mt] = turn + 1;
        scores[i] = quickeval();
        b[m.r][m.c] = turn + 1;
        b[m.r + delta][m.c + m.mt] = old;
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
