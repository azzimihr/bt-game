#pragma once

#include <cstdint>
#include <tuple>
#include <algorithm>
#include <print>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
using namespace std;
using hrc = chrono::steady_clock;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

u64 overwrites, global_depth, searched = 0;

inline auto bb(const u8 b[8][8]) {
    u64 p1 = 0;
    u64 p2 = 0;
    for (int i = 7; i >= 0; --i) {
        for (int j = 7; j >= 0; --j) {
            p1 = (p1 << 1) | (b[i][j] & 1);
            p2 = (p2 << 1) | ((b[i][j] >> 1) & 1);
        }
    }
    return tuple{p1, p2};
}

inline u32 packt(u32 uphash, i8 score, u64 ndepth, u64 flag, u64 best_move_idx, u64 generation) {
    u32 result = 0;
    result |= (u32) (best_move_idx * 3 + flag);
    
    result |= ndepth/2 << 7;

    result |= ((u32)(score + 84) * 3 + generation) << 9;

    result |= uphash << 18;
    return result;
}

inline auto upackt(u32 tt_int) {
    u32 combined = tt_int & 0x7f;
    u8 flag = combined % 3;
    u8 best_move_idx = combined / 3;

    u8 depth = (tt_int >> 7) & 0x3;

    u32 score_gen = (tt_int >> 9) & 0x1ff;
    u32 generation = score_gen % 3;
    i16 score = (score_gen / 3) - 84;

    u32 uphash = (tt_int >> 18);
    return tuple {uphash, score, depth, flag, best_move_idx, generation};
}