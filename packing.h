#pragma once

#include <cstdint>
#include <tuple>
#include <algorithm>
#include <print>
#include <atomic>
#include <thread>
#include <vector>
using namespace std;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;


u64 overwrites, global_depth;

constexpr i64 bmems=2;
struct alignas(4*bmems) Bucket{
    u32 entries[bmems];
};

u64 searched = 0;
constexpr u64 tts = 10;
Bucket* TT = nullptr;

inline void init_tt() {
    if (!TT) {
        TT = new Bucket[1 << tts]();
    }
}
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

inline u32 pack_tt32(u32 uphash, i8 score, u64 ndepth, u64 flag, u64 best_move_idx, u64 generation) {
    u32 result = 0;
    result |= best_move_idx * 3 + flag;
    result |= ndepth/2 << 7;
    result |= (u32)(score + 64) << 9;
    result |= generation << 16;
    result |= uphash << 18;
    return result;
}

inline auto unpack_tt32(u32 tt_int) {
    u32 combined = tt_int & 0x7f;
    u8 flag = combined % 3;
    u8 best_move_idx = combined / 3;
    u8 depth = (tt_int >> 7) & 0x3;
    i16 score = ((i16)((tt_int >> 9) & 0x7f) - 64);
    u32 generation = (tt_int >> 16) & 0x3;
    u32 uphash = (tt_int >> 18);
    return tuple {uphash, score, depth, flag, best_move_idx, generation};
}

inline u64 pack_tt(u64 uphash, i16 score, u64 ndepth, u64 flag, u64 best_move, u64 generation) {
    u64 result = 0;
    result |= best_move;
    result |= flag << 8;
    result |= ndepth/2 << 16;
    result |= generation << 24;
    result |= (u64)(score + 32768) << 32;
    result |= uphash << 48;
    return result;
}

inline auto unpack_tt(u64 tt_int) {
    u16 best_move = tt_int & 0xff;
    u8 flag = (tt_int >> 8) & 0xff;
    u8 depth = (tt_int >> 16) & 0xff;
    u32 generation = (tt_int >> 24) & 0xff;
    i16 score = ((i16)((tt_int >> 32) & 0xffff) - 32768);
    u32 uphash = (tt_int >> 48) & 0x3fff;
    return tuple{uphash, score, depth, flag, best_move, generation};
}

// inline auto bucket1(u64 hash) {
//     u32 ret = 0xFFFFFFFF;
//     const u32 target_tag = hash & 0x7fff;
//     for (i32 i = 0; i < bmems; i++) {
//         u32 entry = TT[hash >> (64 - tts)].entries[i];
//         u32 tag = (entry >> 17) & 0x7fff;
//         if (tag == target_tag) {
//             return entry;
//         }
//     }
//     return ret;
// }



// auto bucket2(u64 hash, i16 score, u64 depth, u64 flag, u64 best_move_idx, u64 generation){
//     auto& B = TT[hash >> (64-tts)];
//     i32 min_d = 999999;
//     i32 idx = bmems;
//     i32 same = bmems;
//     i32 genc = bmems;
//     u64 hs = hash & 0x7fff;

//     for(i32 i=0; i<bmems-1; i++){
//         auto [uphash_, d2, depth_, flag_, d3, gen_] = unpack_tt32(B.entries[i]);
//         idx = (depth_>min_d) ? i : idx;
//         min_d = (depth_>min_d) ? depth_ : min_d;
//         genc = (gen_!=generation) ? i : genc; 
//         same = (uphash_ == hs) ? i : same; 
//     }

//     auto write = pack_tt32(hs, score, depth, flag, best_move_idx, generation);

//     idx = (genc!=bmems) ? genc : idx;
//     idx = (same!=bmems) ? same : idx;
//     volatile u32 pull = B.entries[0];
//     write = (idx==bmems) ? pull : write;
//     idx = (idx==bmems) ? 0 : idx;
//     B.entries[idx] = write;
// };

