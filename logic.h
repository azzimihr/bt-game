#pragma once

#include "score.h"

void setter(u8 d){
        for (u64 i=0; i<24; i+=2){
        *(u64*)&alm[d][i] |= (i<<8) + ((i+1) << 40);
    }
};

auto bMoves(u64 p1, u64 p2, u8 turn, u8 d) {
    u64 acn = ~(p1 | p2);
    u64 pL, pR, pC, pn;    
    i64 total = 0;

    if (turn){
        pn = ~p2;
        pL = (p2 & (pn >> 7)) & 0xfefefefefefefefeULL;
        pR = (p2 & (pn >> 9)) & 0x7f7f7f7f7f7f7f7fULL;
        pC = p2 & (acn >> 8);

        auto minp = min(__builtin_popcountll(pL), __builtin_popcountll(pR));
        for (int i=0; i<minp; i++){

            int pos = 63 - __builtin_clzll(pR);
            pR &= ~(1ull << pos);
            alm[d][total] = {1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;

            pos = 63 - __builtin_clzll(pL);
            pL &= ~(1ull << pos);
            alm[d][total] = {-1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }

        u64 temp = pR;
        while (temp) {
            int pos = 63 - __builtin_clzll(temp);
            temp &= ~(1ull << pos);
            alm[d][total] = {1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }
        
        temp = pL;
        while (temp) {
            int pos = 63 - __builtin_clzll(temp);
            temp &= ~(1ull << pos);
            alm[d][total] = {-1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }
        
        temp = pC;
        while (temp) {
            int pos = 63 - __builtin_clzll(temp);
            temp &= ~(1ull << pos);
            alm[d][total] = {0, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }
    } else {
        pn = ~p1;
        pL = (p1 & (pn << 9)) & 0xfefefefefefefefeULL;
        pR = (p1 & (pn << 7)) & 0x7f7f7f7f7f7f7f7fULL;
        pC = p1 & (acn << 8);

        auto minp = min(__builtin_popcountll(pL), __builtin_popcountll(pR));
        for (int i=0; i<minp; i++){

            int pos = __builtin_ctzll(pL);
            pL &= pL - 1;
            alm[d][total] = {-1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;

            pos = __builtin_ctzll(pR);
            pR &= pR - 1;
            alm[d][total] = {1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }
        
        u64 temp = pR;
        while (temp) {
            int pos = __builtin_ctzll(temp);
            temp &= temp - 1;
            alm[d][total] = {1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }
        
        temp = pL;
        while (temp) {
            int pos = __builtin_ctzll(temp);
            temp &= temp - 1;
            alm[d][total] = {-1, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }
        
        temp = pC;
        while (temp) {
            int pos = __builtin_ctzll(temp);
            temp &= temp - 1;
            alm[d][total] = {0, 0, (i8)(pos / 8), (i8)(pos % 8)};
            total++;
        }
        
    }
    
    setter(d);
    return total;
}



inline auto accesser2(u64 h0, u8 ndepth){
    u64 idx = h0 & ((1ull << tt3) - 1);
    return unpack_tt32(TT3[idx]);
}

i16 mm(u8 depth, u8 maximize, i16 alfa0, i16 beta0, u8 deep, i16 score0, u64 h0, u64 p10, u64 p20, i16 mtd0) {
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
    u8 ndepth = depth - 1;
    auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = accesser2(h0, ndepth);
    u64 idx = h0 & ((1ull << tt3) - 1);

    if (uphash == (h0 >> 50)) {
        // cout << "MATCH!\n";
        if (depth_tt == ndepth/2) {
            if (flag_tt == 0) return score_tt;
            else if (flag_tt == 1 && score_tt <= alfa) return score_tt;
            else if (flag_tt == 2 && score_tt >= beta) return score_tt;
        }
    }

    
    
    auto tm = bMoves(p10, p20, maximize, ndepth);
    
    u8 bm = uphash == (h0 >> 50);
    best_move_tt = bm ? best_move_tt : 0;
    swap(alm[ndepth][best_move_tt], alm[ndepth][0]);
    if (deep < 5) {
        sorter2(maximize, ndepth, bm, tm, score0, p10, p20);
    }
    
    i8 scache[64];
    memset(scache, 100, sizeof(scache));
    i16 val, score;
    i16 st = score0 - mtd0;
    if (maximize) {
        i16 maxeval = -127;
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            mt = (mt==1) ? 1 : (mt==-1) ? -1 : 0;
            i8 r2 = r + 1;
            i8 c2 = c+mt;
            u64 h = h0;
            u8 old = bt[r2][c2];
            
            i16 ev;
            if (r2 == 7) {
                ev = 63;
            } else {
                h ^= zobra[2][r][c];
                h ^= zobra[0][r][c];
                h ^= zobra[old][r2][c2];
                h ^= zobra[2][r2][c2];
                // __builtin_prefetch(&TT[h >> (64 - tts)], 0, 3);
                u8 pos = 8*r + c;
                u64 p2 = p20 & ~(1ull << (pos)) | (1ull << (8*r2 + c2));
                u64 p1 = p10 & ~(1ull << (8*r2 + c2));
                score = st - td2(r, c);
                if (old != 0) {
                    score -= td2(r2, c2);
                }
                bt[r][c] = 0;
                bt[r2][c2] = 2;
                i16 mtd1 = mtd(p1, p2);
                score = score + td2(r2, c2) + mtd1;
                score = clamp(score, (i16)-62, (i16)62);
                searched ++;
                if (ndepth == 0) {
                    ev = score;
                } else {
                    
                    ev = mm(ndepth, 0, alfa, beta, deep+1, score, h, p1, p2, mtd1);
                }
                bt[r][c] = 2;
                bt[r2][c2] = old;
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
            mt = (mt==1) ? 1 : (mt==-1) ? -1 : 0;
            i8 r2 = r - 1;
            
            u64 h = h0;
            i8 c2 = c + mt;
            u8 old = bt[r2][c2];
            i16 ev;
            if (r2 == 0) {
                ev = -63;
            } else {
                
                h ^= zobra[1][r][c];
                h ^= zobra[0][r][c];
                h ^= zobra[old][r2][c2];
                h ^= zobra[1][r2][c2];
                u8 pos = 8*r + c;
                u64 p1 = p10 & ~(1ull << (pos)) | (1ull << (8*r2 + c2));
                u64 p2 = p20 & ~(1ull << (8*r2 + c2));

                score = st - td2(r, c);
                
                if (old != 0) {
                    score -= td2(r2, c2);
                }
                bt[r][c] = 0;
                bt[r2][c2] = 1;
                
                
                i16 mtd1 = mtd(p1, p2);
                score = score + td2(r2, c2) + mtd1;
                score = clamp(score, (i16)-62, (i16)62);
                
                searched ++;
                if (ndepth == 0) {
                    ev = score;
                } else {
                    ev = mm(ndepth, 1, alfa, beta, deep+1, score, h, p1, p2, mtd1);
                }
                bt[r][c] = 1;
                bt[r2][c2] = old;
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
    if ((gen!=gen_tt || ndepth/2>depth_tt || (ndepth/2==depth_tt and (not maximize)))){
        TT3[idx] = pack_tt32(h0 >> 50, val, ndepth, flag, best_move, gen);
    }
    return val;
}

atomic<i16> ALFA;
i16 minimax32(u8 depth, u8 maximize, i16 alfa0, i16 beta0, u8 deep, i16 score0, u64 h0, u64 p10, u64 p20) {
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
    u8 ndepth = depth - 1;
    auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = accesser2(h0, ndepth);
    u64 idx = h0 & ((1ull << tt3) - 1);

    if (uphash == (h0 >> 50)) {
        // cout << "MATCH!\n";
        if (depth_tt == ndepth/2) {
            if (flag_tt == 0) return score_tt;
            else if (flag_tt == 1 && score_tt <= alfa) return score_tt;
            else if (flag_tt == 2 && score_tt >= beta) return score_tt;
        }
    }

    
    
    auto tm = bMoves(p10, p20, maximize, ndepth);
    
    u8 bm = uphash == (h0 >> 50);
    best_move_tt = bm ? best_move_tt : 0;
    swap(alm[ndepth][best_move_tt], alm[ndepth][0]);
    if (ndepth>1) {
        sorter2(maximize, ndepth, bm, tm, score0, p10, p20);
    }
    
    thread_local i8 scache[64];
    memset(scache, 100, sizeof(scache));
    i16 val;
    i16 score;
    if (maximize) {
        i16 maxeval = -127;
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            mt = (mt==1) ? 1 : (mt==-1) ? -1 : 0;
            i8 r2 = r + 1;
            i8 c2 = c+mt;
            u64 h = h0;
            u8 old = bt[r2][c2];
            
            i16 ev;
            if (r2 == 7) {
                ev = 63;
            } else {
                h ^= zobra[2][r][c];
                h ^= zobra[0][r][c];
                h ^= zobra[old][r2][c2];
                h ^= zobra[2][r2][c2];
                u64 idx = h0 & ((1ull << tt3) - 1);
                __builtin_prefetch(&TT3[idx], 0, 3);
                u8 pos = 8*r + c;
                u64 p2 = p20 & ~(1ull << (pos)) | (1ull << (8*r2 + c2));
                u64 p1 = p10 & ~(1ull << (8*r2 + c2));
                auto sc = scache[pos];
                if (sc==100){
                    score = score0 - temp_diff(r, c, p10, p20);
                    scache[pos] = score;
                } else {
                    score = sc;
                }
                if (old != 0) {
                    score -= temp_diff(r2, c2, p10, p20);
                }
                bt[r][c] = 0;
                bt[r2][c2] = 2;
                score = score + temp_diff(r2, c2, p1, p2);
                score = clamp(score, (i16)-62, (i16)62);
                
                // searched ++;
                if (ndepth == 0) {
                    ev = score;
                } else {
                    
                    ev = minimax32(ndepth, 0, alfa, beta, deep+1, score, h, p1, p2);
                }
                bt[r][c] = 2;
                bt[r2][c2] = old;
            }
            
            maxeval = (ev > maxeval) ? ev : maxeval;
            best_move = (ev > maxeval && oi<=42) ? oi : best_move;
            alfa = std::max(alfa, ev);
            if (beta <= alfa) break;
        }
        val = maxeval;
    } else {
        i16 mineval = 127;
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            mt = (mt==1) ? 1 : (mt==-1) ? -1 : 0;
            i8 r2 = r - 1;
            
            u64 h = h0;
            i8 c2 = c + mt;
            u8 old = bt[r2][c2];
            i16 ev;
            if (r2 == 0) {
                ev = -63;
            } else {
                h ^= zobra[1][r][c];
                h ^= zobra[0][r][c];
                h ^= zobra[old][r2][c2];
                h ^= zobra[1][r2][c2];
                u64 idx = h0 & ((1ull << tt3) - 1);
                __builtin_prefetch(&TT3[idx], 0, 3);
                
                u8 pos = 8*r + c;
                u64 p1 = p10 & ~(1ull << (pos)) | (1ull << (8*r2 + c2));
                u64 p2 = p20 & ~(1ull << (8*r2 + c2));
                auto sc = scache[pos];
                if (sc==100){
                    score = score0 - temp_diff(r, c, p10, p20);
                    scache[pos] = score;
                } else {
                    score = sc;
                }

                
                if (old != 0) {
                    score -= temp_diff(r2, c2, p10, p20);
                }
                bt[r][c] = 0;
                bt[r2][c2] = 1;
                
                
                score = score + temp_diff(r2, c2, p1, p2);
                score = clamp(score, (i16)-62, (i16)62);
                
                // searched ++;
                if (ndepth == 0) {
                    ev = score;
                } else {
                    ev = minimax32(ndepth, 1, alfa, beta, deep+1, score, h, p1, p2);
                }
                bt[r][c] = 1;
                bt[r2][c2] = old;
            }
            
            mineval = (ev < mineval) ? ev : mineval;
            best_move = (ev < mineval && oi<=42) ? oi : best_move;
            beta = std::min(beta, ev);
            if (deep==1) alfa = (i16) ALFA;
            if (beta <= alfa) break;
        }
        val = mineval;
    }
    
    u8 flag = (val <= alfa0) ? 1 : (val >= beta0) ? 2 : 0;
    // best_move = (best_move > 31) ? 0 : best_move;
    auto [uphash2, score_tt2, depth_tt2, flag_tt2, best_move_tt2, gen_tt2] = accesser2(h0, ndepth);
    if (((gen!=gen_tt2 and ndepth+4>=2*depth_tt2) || ndepth/2>depth_tt2 || (ndepth/2==depth_tt2 and (not maximize)))){
        TT3[idx] = pack_tt32(h0 >> 50, val, ndepth, flag, best_move, gen);
    }
    return val;
}

alignas(64) u8 boards[48][8][8];
alignas(64) i16 states[48];
alignas(64) u64 zhs[48];
alignas(64) u64 p1s[48];
alignas(64) u64 p2s[48];
alignas(64) atomic<i16> results[48];
constexpr int tc = 6;
array<thread, tc> workers;

atomic<u64> needed;

void task(int id, u8 ndepth, i64 tm) {
    for (int i=id; i<tm; i=needed-1){
        memcpy(bt, &boards[i][0][0], 64);
        i16 ev = minimax32(ndepth, 0, ALFA, 127, 1, states[i], zhs[i], p1s[i], p2s[i]);
        ALFA = std::max((i16) ALFA, ev);
        results[i] = ev;
        needed++;
    }
}

void ai_turn(u8 dept, u8& r1, u8& c1, u8& r2, u8& c2) {
    
    constexpr u8 depth = 8;
    global_depth = depth;
    overwrites = 0;
    u8 win = 8;
    gen = (gen+1)%4;
    
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
        ALFA = -127;
        
        u8 ndepth = depth - 1;
        auto [p10, p20] = bb(b);
        
        auto tm = bMoves(p10, p20, 1, ndepth);

        u64 h = zh();
        auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = accesser2(h, ndepth);
        
        u8 bm = uphash == (h >> 50);
        best_move_tt = bm ? best_move_tt : 0;
        swap(alm[ndepth][best_move_tt], alm[ndepth][0]);
        
        sorter2(1, ndepth, bm, tm, state(), p10, p20);
        
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
            
            println("{}", i);
            memcpy(&boards[i][0][0], b, 64);
            states[i] = state();
            zhs[i] = zh();
            p1s[i] = p1;
            p2s[i] = p2;
            
            b[r][c] = 2;
            b[r2_move][c2_move] = old;
        }
        println("PASSED COLLECTING");

        // best move first
        // memcpy(bt, boards, 64);
        // i16 ev = minimax32(ndepth, 0, ALFA, 127, 1, states[0], zhs[0], p1s[0], p2s[0]);
        // ALFA = std::max((i16)ALFA, ev);
        // results[0] = ev;
        needed = tc;
        for (int i = 0; i < tc; i++){
            workers[i] = thread(task, i, ndepth, tm);
        }
        
        for (thread& t : workers){
            t.join();
        }
        
        i16 high = -300;
        int play = 0;
        for (int i = 0; i < tm; i++){
            if (results[i] > high){
                high = results[i];
                play = i;
            }
        }

        
        auto [mt5, oi5, r5, c5] = alm[ndepth][play];

        r1 = r5;
        c1 = c5;
        r2 = r5+1;
        c2 = c5+mt5;
    }
}
