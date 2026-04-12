#pragma once

#include "score.h"


void setter(u8 d){
        for (u64 i=0; i<24; i+=2){
        *(u64*)&alm[d][i] |= (i<<8) + ((i+1) << 40);
    }
};

inline auto clz(u64& p, i64& total, i8 mt, u8 depth){
    int pos = 63 - __builtin_clzll(p);
    p &= ~(1ull << pos);
    alm[depth][total] = {mt, 0, (i8)(pos / 8), (i8)(pos % 8)};
    total++;
};

inline auto ctz(u64& p, i64& total, i8 mt, u8 depth){
    int pos = __builtin_ctzll(p);
    p &= p - 1;
    alm[depth][total] = {mt, 0, (i8)(pos / 8), (i8)(pos % 8)};
    total++;
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
            clz(pR, total, 1, d);
            clz(pL, total, -1, d);
        }
        while (pR) clz(pR, total, 1, d);
        while (pL) clz(pL, total, -1, d);
        while (pC) clz(pC, total, 0, d);
        
    } else {
        pn = ~p1;
        pL = (p1 & (pn << 9)) & 0xfefefefefefefefeULL;
        pR = (p1 & (pn << 7)) & 0x7f7f7f7f7f7f7f7fULL;
        pC = p1 & (acn << 8);

        auto minp = min(__builtin_popcountll(pL), __builtin_popcountll(pR));
        for (int i=0; i<minp; i++){
            ctz(pR, total, 1, d);
            ctz(pL, total, -1, d);
        }
        while (pR) ctz(pR, total, 1, d);
        while (pL) ctz(pL, total, -1, d);
        while (pC) ctz(pC, total, 0, d);
    }
    
    setter(d);
    return total;
}

inline auto zobrer(u64 h, u8 old, i8 r, i8 c, i8 r2, i8 c2, u8 player){
    h ^= zobra[player][r][c];
    h ^= zobra[0][r][c];
    h ^= zobra[old][r2][c2];
    h ^= zobra[player][r2][c2];
    __builtin_prefetch(&TT3[h & ((1ull << tt3) - 1)], 0, 3);
    return h;
}

auto accesser2(u64 h0){
    u64 idx = h0 & ((1ull << tt3) - 1);
    u32 h = TT3[idx];
    return upackt(h);
}

inline auto writer(u64 h0, i8 val, u64 ndepth, u64 flag, u64 best_move, u8 maximize, u64 idx){
    auto [uphash2, score_tt2, depth_tt2, flag_tt2, best_move_tt2, gen_tt2] = accesser2(h0);
    if (ndepth < 8 and
        ((gen!=gen_tt2 and ndepth+4>=2*depth_tt2)
        || ndepth/2>depth_tt2
        || (ndepth/2==depth_tt2 and (not maximize))))
    {
        TT3[idx] = packt(h0 >> 50, val, ndepth, flag, best_move, gen);
    }
}


atomic<i16> ALFA;
i16 minimax(u8 depth, u8 maximize, i16 alfa0, i16 beta0, u8 deep, i16 score0, u64 h0, u64 p10, u64 p20) {
    i16 alfa = alfa0, beta = beta0;
    u8 best_move = 0;
    u8 ndepth = depth - 1;
    
    auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = accesser2(h0);
    
    u8 bm = uphash == (h0 >> 50) and uphash!=0;
    
    u64 idx = h0 & ((1ull << tt3) - 1);
    auto tm = bMoves(p10, p20, maximize, ndepth);
    
    best_move_tt = bm ? best_move_tt : 0;
    swap(alm[ndepth][best_move_tt], alm[ndepth][0]);
    if (ndepth>2 || (ndepth>1 && !bm)) {
        sorter2(maximize, ndepth, bm, tm, score0, p10, p20);
    }
    
    i16 val;
    i16 score;;
    u64 zbrs[64];
    if (ndepth!=0){
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            i8 r2 = r + (maximize ? 1 : -1);
            i8 c2 = c+mt;
            u8 old = bt[r2][c2];
            zbrs[i] = zobrer(h0, old, r, c, r2, c2, maximize+1);
        }
    }
    if (maximize){
        i16 maxeval = -84;
        i16 bc = -84;
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            i16 ev;
            if (r == 6) {
                ev = 84;
            } else {
                if (ndepth == 0) {
                    i8 r2 = r +1;
                    i8 c2 = c + mt;
                    u8 old = bt[r2][c2];
                    u8 pos = 8*r + c;
                    u8 pos2 = 8*r2 + c2;
                    u64 p2 = p20 & ~(1ull << pos) | (1ull << pos2);
                    u64 p1 = p10 & ~(1ull << pos2);
                    score = score0 - temp_diff(r, c, p10, p20, 2);
                    score = score + temp_diff(r2, c2, p1, p2, 2);
                    if (old != 0) {
                        score = score - temp_diff(r2, c2, p10, p20, 1);
                    }
                    score = clamp(score, (i16)-83, (i16)83);
                    ev = score;
                } else {
                    auto [UPHASH, SCORE, DEPTH, FLAG, BEST, GEN] = accesser2(zbrs[i]);
                    u8 BM = UPHASH == (zbrs[i] >> 50) and UPHASH!=0;
                    if (DEPTH == ndepth/2 and FLAG == 0 and BM) {
                        ev = SCORE;
                    } else {
                        
                        i8 r2 = r +1;
                        i8 c2 = c + mt;
                        u8 old = bt[r2][c2];
                        u8 pos = 8*r + c;
                        u8 pos2 = 8*r2 + c2;
                        u64 p2 = p20 & ~(1ull << pos) | (1ull << pos2);
                        u64 p1 = p10 & ~(1ull << pos2);
                        score = score0 - temp_diff(r, c, p10, p20, 2);
                        score = score + temp_diff(r2, c2, p1, p2, 2);
                        if (old != 0) {
                            score = score - temp_diff(r2, c2, p10, p20, 1);
                        }
                        score = clamp(score, (i16)-83, (i16)83);
                        ev = score;

                        if (FLAG == 1 && SCORE <= alfa and BM) ev = SCORE;
                        else if (FLAG == 2 && SCORE >= beta and BM) ev = SCORE;
                        else{
                            bt[r][c] = 0;
                            bt[r2][c2] = 2;
                            ev = minimax(ndepth, 0, alfa, beta, deep+1, score, zbrs[i], p1, p2);
                            bt[r][c] = 2;
                            bt[r2][c2] = old;
                        }
                    }
                }
            }
            best_move = (ev > maxeval  && oi<=41) ? oi : best_move;
            maxeval = max(ev, maxeval);
            alfa = max(alfa, ev);
            if (beta <= alfa) break;
        }
        val = maxeval;
    } else {
        i16 mineval = 84;
        i16 bc = 84;
        for (int i = 0; i < tm; ++i) {
            auto [mt, oi, r, c] = alm[ndepth][i];
            i16 ev;
            if (r == 1) {
                ev = -84;
            } else {        
                if (ndepth == 0) {
                    i8 r2 = r - 1;
                    i8 c2 = c + mt;
                    u8 old = bt[r2][c2];
                    u8 pos = 8*r + c;
                    u8 pos2 = 8*r2 + c2;
                    u64 p2 = p20 & ~(1ull << pos2);
                    u64 p1 = p10 & ~(1ull << pos) | (1ull << pos2);
                    score = score0 - temp_diff(r, c, p10, p20, 1);
                    score = score + temp_diff(r2, c2, p1, p2, 1);
                    if (old != 0) {
                        score = score - temp_diff(r2, c2, p10, p20, 2-0);
                    }
                    score = clamp(score, (i16)-83, (i16)83);
                    ev = score;
                } else {
                    if (deep==3) {
                        auto [UPHASH, SCORE, DEPTH, FLAG, BEST, GEN] = accesser2(h0);
                        u8 BM = UPHASH == (h0 >> 50) and UPHASH!=0;
                        if (DEPTH == ndepth/2) {
                            if (FLAG == 0) return SCORE;
                            else if (FLAG == 1 && SCORE <= alfa0) return SCORE;
                            else if (FLAG == 2 && SCORE >= beta0) return SCORE;
                        }
                    }
                    
                    auto [UPHASH, SCORE, DEPTH, FLAG, BEST, GEN] = accesser2(zbrs[i]);
                    u8 BM = UPHASH == (zbrs[i] >> 50) and UPHASH!=0;
                    if (DEPTH == ndepth/2 and FLAG == 0 and BM) {
                        ev = SCORE;
                    } else {
                        
                        i8 r2 = r - 1;
                        i8 c2 = c + mt;
                        u8 old = bt[r2][c2];
                        u8 pos = 8*r + c;
                        u8 pos2 = 8*r2 + c2;
                        u64 p2 = p20 & ~(1ull << pos2);
                        u64 p1 = p10 & ~(1ull << pos) | (1ull << pos2);
                        score = score0 - temp_diff(r, c, p10, p20, 1);
                        score = score + temp_diff(r2, c2, p1, p2, 1);
                        if (old != 0) {
                            score = score - temp_diff(r2, c2, p10, p20, 2);
                        }
                        score = clamp(score, (i16)-83, (i16)83);
                        ev = score;

                        if (FLAG == 1 && SCORE <= alfa and BM) ev = SCORE;
                        else if (FLAG == 2 && SCORE >= beta and BM) ev = SCORE;
                        else{
                            bt[r][c] = 0;
                            bt[r2][c2] = 1;
                            ev = minimax(ndepth, 1, alfa, beta, deep+1, score, zbrs[i], p1, p2);
                            bt[r][c] = 1;
                            bt[r2][c2] = old;
                        }
                    }
                }
            }
            
            best_move = (ev < mineval  && oi<=41) ? oi : best_move;

            mineval = min(ev, mineval);
            beta = min(beta, ev);
            
            if (deep==1) alfa = max(alfa, (i16) ALFA);
            if (beta <= alfa) break;
        }
        val = mineval;
    }
    
    u8 flag = (val <= alfa0) ? 1 : (val >= beta0) ? 2 : 0;
    writer(h0, val, ndepth, flag, best_move, maximize, idx);
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
double total_work = 0;
double working_time = 0;

void task(int id, u8 ndepth, i64 tm, hrc::time_point st) {
    for (int i=id; i<tm; i=needed.fetch_add(1)){
        memcpy(bt, &boards[i][0][0], 64);
        i16 ev = minimax(ndepth, 0, ALFA, 84, 1, states[i], zhs[i], p1s[i], p2s[i]);
        ALFA = std::max((i16) ALFA, ev);
        results[i] = ev;
        // this_thread::sleep_for(0.002s);
    }
    total_work += chrono::duration_cast<chrono::microseconds>(hrc::now() - st).count();

}

void ai_turn(u8 depth, u8& r1, u8& c1, u8& r2, u8& c2, hrc::time_point st) {
    global_depth = depth;
    overwrites = 0;
    u8 win = 8;
    gen = (gen+1)%3;
    
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
        ALFA = -84;
        
        u8 ndepth = depth - 1;
        auto [p10, p20] = bb(b);
        
        auto tm = bMoves(p10, p20, 1, ndepth);

        u64 h = zh();
        auto [uphash, score_tt, depth_tt, flag_tt, best_move_tt, gen_tt] = accesser2(h);
        
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

        needed = 0;
        for (int i = 0; i < tc; i++){
            workers[i] = thread(task, i, ndepth, tm, st);
        }
        
        for (thread& t : workers){
            t.join();
        }
        working_time += chrono::duration_cast<chrono::microseconds>(hrc::now() - st).count();
        println("{} {}", minscore, maxscore);
        
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
