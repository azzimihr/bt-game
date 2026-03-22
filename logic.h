#pragma once

#include <algorithm>
#include <print>

#include "score.h"


u64 zh() {
    u64 h = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            h ^= zobra[b[i][j]][i][j];
        }
    }
    return h;
}

void bmoves2(u64 p1, u64 p2, u8 turn, u8 d) {
    u64 acn = ~(p1 | p2);

    u64 pn0 = ~p1;
    u64 pL_t0 = (p1 & (pn0 << 9)) & 0xfefefefefefefefe;
    u64 pR_t0 = (p1 & (pn0 << 7)) & 0x7f7f7f7f7f7f7f7f;
    u64 pC_t0 = p1 & (acn << 8);
    
    u64 pn1 = ~p2;
    u64 pL_t1 = (p2 & (pn1 >> 7)) & 0xfefefefefefefefe;
    u64 pR_t1 = (p2 & (pn1 >> 9)) & 0x7f7f7f7f7f7f7f7f;
    u64 pC_t1 = p2 & (acn >> 8);
    
    u64 pL = turn ? pL_t1 : pL_t0;
    u64 pR = turn ? pR_t1 : pR_t0;
    u64 pC = turn ? pC_t1 : pC_t0;
    
    u8 total = 0;
    u8 last = 0;
    
    for (int i = 1; i < 8; ++i) {
        int i_actual = (7 - 2 * i) * turn + i;
        int it8 = 8 * i_actual;
        for (int j = 0; j < 8; ++j) {
            int j1 = j & 1;
            int j_actual = 7 * j1 + (j / 2) * (1 - (2 * j1));
            int m = it8 + j_actual;
            
            u8 iL = (pL >> m) & 1;
            u8 iR = (pR >> m) & 1;
            u8 iC = (pC >> m) & 1;
            
            last = iR ? pack(i_actual, j_actual, 1) : last;
            allm[d][total] = last;
            total += iR;
            
            last = iL ? pack(i_actual, j_actual, -1) : last;
            allm[d][total] = last;
            total += iL;

            last = iC ? pack(i_actual, j_actual, 0) : last;
            allm[d][total] = last;
            total += iC;
            
        }
    }

    allm[d][48] = total;
}

void bmoves(u64 p1, u64 p2, u8 turn, u8 d) {
    u64 acn = ~(p1 | p2);

    u64 pn0 = ~p1;
    u64 pL_t0 = (p1 & (pn0 << 9)) & 0xfefefefefefefefe;
    u64 pR_t0 = (p1 & (pn0 << 7)) & 0x7f7f7f7f7f7f7f7f;
    u64 pC_t0 = p1 & (acn << 8);
    
    u64 pn1 = ~p2;
    u64 pL_t1 = (p2 & (pn1 >> 7)) & 0xfefefefefefefefe;
    u64 pR_t1 = (p2 & (pn1 >> 9)) & 0x7f7f7f7f7f7f7f7f;
    u64 pC_t1 = p2 & (acn >> 8);
    
    u64 pL = turn ? pL_t1 : pL_t0;
    u64 pR = turn ? pR_t1 : pR_t0;
    u64 pC = turn ? pC_t1 : pC_t0;
    
    u8 total = 0;
    
    u64 temp = pR;
    while (temp) {
        int pos = __builtin_ctzll(temp);
        temp &= temp - 1;
        int i_actual = pos / 8;
        int j_actual = pos % 8;
        allm[d][total] = pack(i_actual, j_actual, 1);
        total++;
    }
    
    temp = pL;
    while (temp) {
        int pos = __builtin_ctzll(temp);
        temp &= temp - 1;
        int i_actual = pos / 8;
        int j_actual = pos % 8;
        allm[d][total] = pack(i_actual, j_actual, -1);
        total++;
    }
    
    temp = pC;
    while (temp) {
        int pos = __builtin_ctzll(temp);
        temp &= temp - 1;
        int i_actual = pos / 8;
        int j_actual = pos % 8;
        allm[d][total] = pack(i_actual, j_actual, 0);
        total++;
    }
    
    allm[d][48] = total;
}

u64 accesser(u64 h0){
    u64 idx = h0 & ((1ull << tt2) - 1);
    u64 ret_val = tt[idx];
    return ret_val;
}

i16 minimax(u8 depth, u8 maximize, i16 alfa0, i16 beta0, u8 deep, i16 score0, u64 h0, u64 p10, u64 p20) {
    maxscore = std::max(maxscore, score0);
    minscore = std::min(minscore, score0);
    i16 alfa = alfa0, beta = beta0;
    u8 best_move = 0;
    
    auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = unpack_tt(accesser(h0));
    u64 idx = h0 & ((1ull << tt2) - 1);
    u8 ndepth = depth - 1;


    println("MATCH!");
    if (uphash == (h0 >> 41)) {
        if (depth_tt == ndepth/2) {
            if (flag_tt == 0) return score_tt;
            else if (flag_tt == 1 && score_tt <= alfa) return score_tt;
            else if (flag_tt == 2 && score_tt >= beta) return score_tt;
        }
    }
    
    bmoves(p10, p20, maximize, ndepth);
    
    u8 bm = 0;
    if (deep < 5) {
        sorter(maximize, ndepth, bm);
    }
    if (best_move_tt != 0 and uphash == (h0 >> 41)) {
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
            auto [r, c, mt] = unpack(allm[ndepth][i]);
            u8 r2 = r + 1;
            
            i16 ev;
            if (r2 == 7) {
                ev = 63;
            } else {
                u8 c2 = (u8)(int(c) + int(mt));
                u8 old = b[r2][c2];
                i16 score = score0 - temp_diff(r, c, p10, p20);

                if (old != 0) {
                    score -= temp_diff(r2, c2, p10, p20);
                }
                
                b[r][c] = 0;
                b[r2][c2] = 2;
                
                u64 p2 = p20 & ~(1ull << (8*r + c)) | (1ull << (8*r2 + c2));
                u64 p1 = p10 & ~(1ull << (8*r2 + c2));
                
                score = score + temp_diff(r2, c2, p1, p2);
                score = clamp(score, (i16)-62, (i16)62);
                
                if (ndepth == 0) {
                    ev = score;
                } else {
                    u64 h = h0;
                    h ^= zobra[2][r][c];
                    h ^= zobra[0][r][c];
                    h ^= zobra[old][r2][c2];
                    h ^= zobra[2][r2][c2];
                    
                    ev = minimax(ndepth, 0, alfa, beta, deep+1, score, h, p1, p2);
                }
                b[r][c] = 2;
                b[r2][c2] = old;
            }
            
            maxeval = (ev > maxeval) ? ev : maxeval;
            best_move = (ev > maxeval) ? allm[ndepth][i] : best_move;
            alfa = std::max(alfa, ev);
            if (beta <= alfa) break;
        }
        val = maxeval;
    } else {
        i16 mineval = 127;
        for (int i = 0; i < allm[ndepth][48]; ++i) {
            auto [r, c, mt] = unpack(allm[ndepth][i]);
            u8 r2 = r - 1;
            
            i16 ev;
            if (r2 == 0) {
                ev = -63;
            } else {
                u8 c2 = (u8)(int(c) + int(mt));
                u8 old = b[r2][c2];
                i16 score = score0 - temp_diff(r, c, p10, p20);

                if (old != 0) {
                    score -= temp_diff(r2, c2, p10, p20);
                }
                
                b[r][c] = 0;
                b[r2][c2] = 1;
                
                u64 p1 = p10 & ~(1ull << (8*r + c)) | (1ull << (8*r2 + c2));
                u64 p2 = p20 & ~(1ull << (8*r2 + c2));

                score = score + temp_diff(r2, c2, p1, p2);
                score = clamp(score, (i16)-62, (i16)62);
                
                if (ndepth == 0) {
                    ev = score;
                } else {
                    u64 h = h0;
                    h ^= zobra[1][r][c];
                    h ^= zobra[0][r][c];
                    h ^= zobra[old][r2][c2];
                    h ^= zobra[1][r2][c2];
                    
                    ev = minimax(ndepth, 1, alfa, beta, deep+1, score, h, p1, p2);
                }
                b[r][c] = 1;
                b[r2][c2] = old;
            }
            
            mineval = (ev < mineval) ? ev : mineval;
            best_move = (ev < mineval) ? allm[ndepth][i] : best_move;
            beta = std::min(beta, ev);
            if (beta <= alfa) break;
        }
        val = mineval;
    }
    
    u8 flag = (val <= alfa0) ? 1 : (val >= beta0) ? 2 : 0;
    if (gen!=gen_tt || ndepth/2>depth_tt || (ndepth/2==depth_tt and (not maximize))){
        tt[idx] = pack_tt(h0 >> 41, val, depth, flag, best_move, gen);
    }
    return val;
}

void ai_turn2(u8 depth, u8& r1, u8& c1, u8& r2, u8& c2) {

    u8 win = 8;
    gen = 1-gen;
    
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
        auto [p10, p20] = bb(b);
        
        bmoves(p10, p20, 1, ndepth);
        u64 h = zh();
        u64 idx = h & ((1ull << tt2) - 1);
        auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = unpack_tt(tt[idx]);
        
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
            auto [r, c, mt] = unpack(allm[ndepth][i]);
            if (b[r][c] != 2) continue;
            
            u8 r2_move = r + 1;
            u8 c2_move = (u8)(int(c) + int(mt));
            
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
    println("{} {}", minscore, maxscore);
}

