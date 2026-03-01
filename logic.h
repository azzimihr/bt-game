#pragma once

#include <algorithm>
#include "packing.h"

struct Move {
    u8 r1, c1, r2, c2;
};

alignas(64) u8 b[8][8];
alignas(64) u8 allm[10][64];
alignas(64) i16 scores[64];

alignas(64) u64 zobra[3][8][8] = {};
alignas(64) constexpr u8 replica[8][8] = {
    {99, 99, 99, 99, 99, 99, 99, 99},
    {10,  9, 10, 10, 10, 10,  9, 10},
    { 8,  7,  8,  8,  8,  8,  7,  8},
    { 6,  5,  6,  6,  6,  6,  5,  6},
    { 5,  4,  5,  5,  5,  5,  4,  5},
    { 4,  3,  4,  4,  4,  4,  3,  4},
    { 3,  2,  3,  3,  3,  3,  2,  3},
    { 3,  2,  3,  3,  3,  3,  2,  3}
};
alignas(64) u64 tt[1 << 28];

u64 zh() {
    u64 h = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            h ^= zobra[b[i][j]][i][j];
        }
    }
    return h;
}

void moves(u8 turn, u8 d) {
    u8 total = 0;
    i8 delta = turn * 2 - 1;
    u8 owned = 1 + turn;
    
    for (int i0 = 0; i0 < 8; ++i0) {
        int i = i0 + (7 - 2 * i0) * turn;
        int pos = i + delta;
        
        for (int j0 = 0; j0 < 8; ++j0) {
            int j1 = j0 & 1;
            int j = 7 * j1 + (j0 / 2) * (1 - (2 * j1));
            
            if (b[i][j] != owned) {
                continue;
            }
            if (j > 0 && b[pos][j - 1] != owned) {
                allm[d][total] = pack(i, j, -1);
                total++;
            }
            if (j < 7 && b[pos][j + 1] != owned) {
                allm[d][total] = pack(i, j, 1);
                total++;
            }
            if (b[pos][j] == 0) {
                allm[d][total] = pack(i, j, 0);
                total++;
            }
        }
    }
    allm[d][6*8] = total;
}
void bmoves(u64 p1, u64 p2, u8 turn, u8 d) {
    u64 acn = ~(p1 | p2);
    u64 pL, pR, pC;
    
    if (!turn) {
        u64 pn = ~p1;
        pL = (p1 & (pn << 9)) & 0xfefefefefefefefe;
        pR = (p1 & (pn << 7)) & 0x7f7f7f7f7f7f7f7f;
        pC = p1 & (acn << 8);
    } else {
        u64 pn = ~p2;
        pL = (p2 & (pn >> 7)) & 0xfefefefefefefefe;
        pR = (p2 & (pn >> 9)) & 0x7f7f7f7f7f7f7f7f;
        pC = p2 & (acn >> 8);
    }
    
    u8 total = 0;
    u8 last = 0;
    i8 delta = turn * 2 - 1;
    
    for (int i = 0; i < 8; ++i) {
        int i_actual = (7 - 2 * i) * turn + i;
        int it8 = 8 * i_actual;
        
        for (int j = 0; j < 8; ++j) {
            int j1 = j & 1;
            int j_actual = 7 * j1 + (j / 2) * (1 - (2 * j1));
            int m = it8 + j_actual;
            
            u8 iL = (pL >> m) & 1;
            u8 iR = (pR >> m) & 1;
            // u8 iC = (pC >> m) & 1;
            
            last *= (1 - iR);
            last += iR * pack(i_actual, j_actual, 1);
            allm[d][total] = last;
            total += iR;
            
            last *= (1 - iL);
            last += iL * pack(i_actual, j_actual, -1);
            allm[d][total] = last;
            total += iL;

            // last *= (1 - iC);
            // last += iC * pack(i_actual, j_actual, 0);
            // allm[d][total] = last;
            // total += iC;
            
        }
    }

    for (int i = 0; i < 8; ++i) {
        int i_actual = (7 - 2 * i) * turn + i;
        int it8 = 8 * i_actual;
        
        for (int j = 0; j < 8; ++j) {
            int j1 = j & 1;
            int j_actual = 7 * j1 + (j / 2) * (1 - (2 * j1));
            int m = it8 + j_actual;

            u8 iC = (pC >> m) & 1;

            last *= (1 - iC);
            last += iC * pack(i_actual, j_actual, 0);
            allm[d][total] = last;
            total += iC;
        }
    }

    allm[d][48] = total;
}

i16 temp_diff(u8 r, u8 c, u8 maximize) {
    u8 val = b[r][c];
    i8 s = 3 - 2 * val;
    
    int idx = (8 - 1) * (val - 1) + s * r;
    i16 h = -s * replica[idx][c];
    
    constexpr int DR[] = {-1, -1, -1, 1, 1, 1};
    constexpr int DC[] = {0, -1, 1, 0, -1, 1};
    
    for (int k = 0; k < 6; ++k) {
        int rr = r + DR[k];
        if (rr >= 0 && rr < 8) {
            int cc = c + DC[k];
            if (cc >= 0 && cc < 8) {
                h += s * (b[rr][cc] == val);
            }
        }
    }
    return h;
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

i16 quickeval() {
    i16 h = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            u8 val = b[i][j];
            if (!val) continue;
            h += (2 * val - 3) * replica[(8 - 1) * (val - 1) - (2 * val - 3) * i][j];
        }
    }
    return h;
}

i16 state() {
    return move_diff() + quickeval();
}

i8 gameover() {
    for (int i = 0; i < 8; ++i) {
        if (b[0][i] == 1) return -127;
        if (b[7][i] == 2) return 127;
    }
    return 0;
}

void sorter(u8 turn, u8 d, u8 bm) {
    u8 total = allm[d][48];
    i8 delta = 2 * turn - 1;
    
    // Evaluate all moves
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
    
    if (turn) {
        for (int i = bm; i < total; ++i) {
            i16 key_s = scores[i];
            u8 key_m = allm[d][i];
            int j = i - 1;
            while (j >= bm && scores[j] < key_s) {
                scores[j + 1] = scores[j];
                allm[d][j + 1] = allm[d][j];
                j--;
            }
            scores[j + 1] = key_s;
            allm[d][j + 1] = key_m;
        }
    } else {
        for (int i = bm; i < total; ++i) {
            i16 key_s = scores[i];
            u8 key_m = allm[d][i];
            int j = i - 1;
            while (j >= bm && scores[j] > key_s) {
                scores[j + 1] = scores[j];
                allm[d][j + 1] = allm[d][j];
                j--;
            }
            scores[j + 1] = key_s;
            allm[d][j + 1] = key_m;
        }
    }
}

i16 minimax(u8 depth, u8 maximize, i16 alfa0, i16 beta0, u8 deep, i16 score0, u64 h0, u64 p10, u64 p20) {
    i16 alfa = alfa0, beta = beta0;
    u8 best_move = 0;
    u64 idx = h0 & ((1ull << 28) - 1);
    
    u64 uphash, score_tt, depth_tt, flag_tt, best_move_tt;
    tie(uphash, score_tt, depth_tt, flag_tt, best_move_tt) = unpack_tt(tt[idx]);
    
    if (uphash == (h0 >> 40)) {
        if (depth_tt >= depth) {
            if (flag_tt == 0) return score_tt;
            else if (flag_tt == 1 && score_tt <= alfa) return score_tt;
            else if (flag_tt == 2 && score_tt >= beta) return score_tt;
        }
    }
    
    u8 ndepth = depth - 1;
    bmoves(p10, p20, maximize, ndepth);
    
    u8 bm = 0;
    if (deep < 5) {
        sorter(maximize, ndepth, bm);
    }
    if (best_move_tt != 0) {
        for (int i = 0; i < allm[ndepth][48]; ++i) {
            if (allm[ndepth][i] == best_move_tt) {
                swap(allm[ndepth][i], allm[ndepth][0]);
                bm = 1;
                break;
            }
        }
    }
    
    i16 val;
    if (maximize) {
        i16 maxeval = -127;
        for (int i = 0; i < allm[ndepth][48]; ++i) {
            u8 r, c;
            i8 mt;
            tie(r, c, mt) = unpack(allm[ndepth][i]);
            u8 r2 = r + 1;
            
            i16 ev;
            if (r2 == 7) {
                ev = 127;
            } else {
                u8 c2 = (u8)(int(c) + int(mt));  // Safe signed addition
                u8 old = b[r2][c2];
                i16 score = score0 - temp_diff(r, c, 1);
                if (old != 0) score -= temp_diff(r2, c2, 1);
                
                b[r][c] = 0;
                b[r2][c2] = 2;
                score = score + temp_diff(r2, c2, 1);
                
                if (ndepth == 0) {
                    ev = score;
                } else {
                    u64 h = h0;
                    h ^= zobra[2][r][c];
                    h ^= zobra[0][r][c];
                    h ^= zobra[old][r2][c2];
                    h ^= zobra[2][r2][c2];
                    
                    u64 p2 = p20 & ~(1ull << (8*r + c)) | (1ull << (8*r2 + c2));
                    u64 p1 = p10 & ~(1ull << (8*r2 + c2));
                    ev = minimax(ndepth, 0, alfa, beta, deep+1, score, h, p1, p2);
                }
                b[r][c] = 2;
                b[r2][c2] = old;
            }
            
            if (ev > maxeval) {
                maxeval = ev;
                best_move = allm[ndepth][i];
            }
            alfa = std::max(alfa, ev);
            if (beta <= alfa) break;
        }
        val = maxeval;
    } else {
        i16 mineval = 127;
        for (int i = 0; i < allm[ndepth][48]; ++i) {
            u8 r, c;
            i8 mt;
            tie(r, c, mt) = unpack(allm[ndepth][i]);
            u8 r2 = r - 1;
            
            i16 ev;
            if (r2 == 0) {
                ev = -127;
            } else {
                u8 c2 = (u8)(int(c) + int(mt));  // Safe signed addition
                u8 old = b[r2][c2];
                i16 score = score0 - temp_diff(r, c, 0);
                if (old != 0) score -= temp_diff(r2, c2, 0);
                
                b[r][c] = 0;
                b[r2][c2] = 1;
                score = score + temp_diff(r2, c2, 0);
                
                if (ndepth == 0) {
                    ev = score;
                } else {
                    u64 h = h0;
                    h ^= zobra[1][r][c];
                    h ^= zobra[0][r][c];
                    h ^= zobra[old][r2][c2];
                    h ^= zobra[1][r2][c2];
                    
                    u64 p1 = p10 & ~(1ull << (8*r + c)) | (1ull << (8*r2 + c2));
                    u64 p2 = p20 & ~(1ull << (8*r2 + c2));
                    ev = minimax(ndepth, 1, alfa, beta, deep+1, score, h, p1, p2);
                }
                b[r][c] = 1;
                b[r2][c2] = old;
            }
            
            if (ev < mineval) {
                mineval = ev;
                best_move = allm[ndepth][i];
            }
            beta = std::min(beta, ev);
            if (beta <= alfa) break;
        }
        val = mineval;
    }
    
    u8 flag = 0;
    if (val <= alfa0) flag = 1;
    else if (val >= beta0) flag = 2;
    if (depth_tt < depth) {
        tt[idx] = pack_tt(h0 >> 40, val, depth, flag, best_move);
    }
    return val;
}

void ai_turn(u8 depth, u8& r1, u8& c1, u8& r2, u8& c2) {
    u8 win = 8;
    
    for (int i = 0; i < 8; ++i) {
        if (b[6][i] == 2) {
            win = i;
            break;
        }
    }
    if (win < 7) {
        r1 = 6;
        c1 = win;
        r2 = 7;
        c2 = win + 1;
    } else if (win == 7) {
        r1 = 6;
        c1 = win;
        r2 = 7;
        c2 = win - 1;
    } else {
        i16 maxeval = -127;
        i16 alfa = -127;
        i16 beta = 127;
        
        u8 ndepth = depth - 1;
        u64 p10, p20;
        tie(p10, p20) = bb(b);
        
        bmoves(p10, p20, 1, ndepth);
        u64 h = zh();
        u64 idx = h & ((1ull << 22) - 1);
        u64 uphash, score_tt, depth_tt, flag_tt, best_move_tt;
        tie(uphash, score_tt, depth_tt, flag_tt, best_move_tt) = unpack_tt(tt[idx]);
        
        u8 bm = 0;
        sorter(1, ndepth, bm);
        if (best_move_tt != 0) {
            for (int i = 0; i < allm[ndepth][48]; ++i) {
                if (allm[ndepth][i] == best_move_tt) {
                    swap(allm[ndepth][i], allm[ndepth][0]);
                    bm = 1;
                    break;
                }
            }
        }
        
        for (int i = 0; i < allm[ndepth][48]; ++i) {
            u8 r, c;
            i8 mt;
            tie(r, c, mt) = unpack(allm[ndepth][i]);
            if (b[r][c] != 2) continue;
            
            u8 r2_move = r + 1;
            u8 c2_move = (u8)(int(c) + int(mt));  // Safe signed addition
            
            u8 old = b[r2_move][c2_move];
            b[r][c] = 0;
            b[r2_move][c2_move] = 2;
            
            u64 p2 = p20 & ~(1ull << (8*r + c)) | (1ull << (8*r2_move + c2_move));
            u64 p1 = p10 & ~(1ull << (8*r2_move + c2_move));
            
            i16 ev = minimax(ndepth, 0, alfa, beta, 1, state(), zh(), p1, p2);
            
            if (ev > maxeval) {
                maxeval = ev;
                r1 = r;
                c1 = c;
                r2 = r2_move;
                c2 = c2_move;
            }
            alfa = std::max(alfa, ev);
            
            b[r][c] = 2;
            b[r2_move][c2_move] = old;
            
            if (beta <= alfa) break;
        }
    }
}

// Wrapper for C FFI that returns Move struct
Move ai_turn_wrapper(u8 depth) {
    Move m = {0, 0, 0, 0};
    ai_turn(depth, m.r1, m.c1, m.r2, m.c2);
    return m;
}