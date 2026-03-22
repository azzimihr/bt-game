#include "logic.h"


auto bMoves(u64 p1, u64 p2, u8 turn, u8 d) {
    u64 acn = ~(p1 | p2);
    
    u64 pn0 = ~p1;
    u64 pn1 = ~p2;
    u64 pL_t0 = (p1 & (pn0 << 9)) & 0xfefefefefefefefe;
    u64 pL_t1 = (p2 & (pn1 >> 7)) & 0xfefefefefefefefe;
    u64 pR_t0 = (p1 & (pn0 << 7)) & 0x7f7f7f7f7f7f7f7f;
    u64 pR_t1 = (p2 & (pn1 >> 9)) & 0x7f7f7f7f7f7f7f7f;
    u64 pC_t0 = p1 & (acn << 8);
    u64 pC_t1 = p2 & (acn >> 8);
    
    u64 pL = turn ? pL_t1 : pL_t0;
    u64 pR = turn ? pR_t1 : pR_t0;
    u64 pC = turn ? pC_t1 : pC_t0;
    
    i64 total = 0;
    
    u64 temp = pR;
    while (temp) {
        int pos = __builtin_ctzll(temp);
        temp &= temp - 1;
        int i = pos / 8;
        int j = pos % 8;
        alm[d][total] = {1, (i8)total, (i8)i, (i8)j};
        total++;
    }
    
    temp = pL;
    while (temp) {
        int pos = __builtin_ctzll(temp);
        temp &= temp - 1;
        int i = pos / 8;
        int j = pos % 8;
        alm[d][total] = {-1, (i8)total, (i8)i, (i8)j};
        total++;
    }
    
    temp = pC;
    while (temp) {
        int pos = __builtin_ctzll(temp);
        temp &= temp - 1;
        int i = pos / 8;
        int j = pos % 8;
        alm[d][total] = {0, (i8)total, (i8)i, (i8)j};
        total++;
    }
    
    return total;
}

inline u32 accesser2(u64 h0){
    u64 idx = h0 & ((1ull << tt3) - 1);
    u32 ret_val = TT3[idx];
    return ret_val;
}

i16 negamax32(u8 depth, u8 maximize, i16 alfa0, i16 beta0, u8 deep, i16 score0, u64 h0, u64 p10, u64 p20) {
    // maxscore = std::max(maxscore, score0);
    // minscore = std::min(minscore, score0);
    i16 alfa = alfa0, beta = beta0;
    u8 best_move = 0;
    
    // auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = unpack_tt32(bucket1(h0));
    
    // u8 ndepth = depth - 1;
    // if (depth_tt == ndepth/2) {
    //     if (flag_tt == 0) return score_tt;
    //     else if (flag_tt == 1 && score_tt <= alfa) return score_tt;
    //     else if (flag_tt == 2 && score_tt >= beta) return score_tt;
    // }
    auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = unpack_tt32(accesser2(h0));
    u64 idx = h0 & ((1ull << tt3) - 1);
    u8 ndepth = depth - 1;

    if (uphash == (h0 >> 49)) {
        // cout << "MATCH!\n";
        if (depth_tt == ndepth/2) {
            if (flag_tt == 0) return score_tt;
            else if (flag_tt == 1 && score_tt <= alfa) return score_tt;
            else if (flag_tt == 2 && score_tt >= beta) return score_tt;
        }
    }

    
    
    auto tm = bMoves(p10, p20, maximize, ndepth);
    
    u8 bm = uphash == (h0 >> 49);
    best_move_tt = bm ? best_move_tt : 0;
    swap(alm[ndepth][best_move_tt], alm[ndepth][0]);
    if (deep < 5) {
        sorter2(maximize, ndepth, bm, tm);
    }
    
    i16 val;
    if (maximize) {
        i16 maxeval = -127;
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            i8 r2 = r + 1;
            i8 c2 = c+mt;
            u64 h = h0;
            u8 old = b[r2][c2];
            h ^= zobra[2][r][c];
            h ^= zobra[0][r][c];
            h ^= zobra[old][r2][c2];
            h ^= zobra[2][r2][c2];

            i16 ev;
            if (r2 == 7) {
                ev = 63;
            } else {
                // __builtin_prefetch(&TT[h >> (64 - tts)], 0, 3);
                i16 score = score0 - temp_diff(r, c, p10, p20);
                auto opt_change = temp_diff(r2, c2, p10, p20);
                if (old != 0) {
                    score -= opt_change;
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
                    
                    ev = negamax32(ndepth, 0, alfa, beta, deep+1, score, h, p1, p2);
                }
                b[r][c] = 2;
                b[r2][c2] = old;
            }
            
            maxeval = (ev > maxeval) ? ev : maxeval;
            best_move = (ev > maxeval && oi<=31) ? oi : best_move;
            alfa = std::max(alfa, ev);
            if (beta <= alfa) break;
        }
        val = maxeval;
    } else {
        i16 mineval = 127;
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            i8 r2 = r - 1;
            
            u64 h = h0;
            i8 c2 = c + mt;
            u8 old = b[r2][c2];
            h ^= zobra[1][r][c];
            h ^= zobra[0][r][c];
            h ^= zobra[old][r2][c2];
            h ^= zobra[1][r2][c2];
            i16 ev;
            if (r2 == 0) {
                ev = -63;
            } else {
                i16 score = score0 - temp_diff(r, c, p10, p20);
                
                auto opt_change = temp_diff(r2, c2, p10, p20);
                if (old != 0) {
                    score -= opt_change;
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
                    
                    ev = negamax32(ndepth, 1, alfa, beta, deep+1, score, h, p1, p2);
                }
                b[r][c] = 1;
                b[r2][c2] = old;
            }
            
            mineval = (ev < mineval) ? ev : mineval;
            best_move = (ev < mineval && oi<=31) ? oi : best_move;
            beta = std::min(beta, ev);
            if (beta <= alfa) break;
        }
        val = mineval;
    }
    
    u8 flag = (val <= alfa0) ? 1 : (val >= beta0) ? 2 : 0;
    // best_move = (best_move > 31) ? 0 : best_move;
    if (gen!=gen_tt || ndepth/2>depth_tt || (ndepth/2==depth_tt and (not maximize))){
        TT3[idx] = pack_tt32(h0 >> 49, val, depth, flag, best_move, gen);
    }
    return val;
}
void ai_turn(u8 depth, u8& r1, u8& c1, u8& r2, u8& c2) {
    global_depth = depth;
    overwrites = 0;
    u8 win = 8;
    gen = (gen+1)%2;
    
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
        
        auto tm = bMoves(p10, p20, 1, ndepth);


        u64 h = zh();
        auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = unpack_tt32(accesser2(h));
        
        u8 bm = uphash == (h >> 49);
        best_move_tt = bm ? best_move_tt : 0;
        swap(alm[ndepth][best_move_tt], alm[ndepth][0]);
        sorter2(1, ndepth, bm, tm);
        
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            if (b[r][c] != 2) continue;
            
            i8 r2_move = r + 1;
            i8 c2_move = c + mt;
            
            u8 old = b[r2_move][c2_move];
            b[r][c] = 0;
            b[r2_move][c2_move] = 2;
            
            u64 p2 = p20 & ~(1ull << (8*r + c)) | (1ull << (8*r2_move + c2_move));
            u64 p1 = p10 & ~(1ull << (8*r2_move + c2_move));
            
            i16 ev = negamax32(ndepth, 0, alfa, beta, 1, state(), zh(), p1, p2);
            
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
            // println("{}",i);
            if (beta <= alfa) break;
        }
        // println("{} {} 23", ndepth, overwrites);
    }
}
