#pragma once

#include <cstdint>
#include <tuple>
#include <algorithm>
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
// struct alignas(4){
    
// }

constexpr i64 bmems=2;
struct alignas(4*bmems) Bucket{
    u32 entries[bmems];
};

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

inline auto pack(u32 r, u32 c, u32 move_type) {
    u32 move_rep = 0;
    move_rep |= r;
    move_rep |= c << 3;
    move_rep |= (move_type + 1) << 6;
    return move_rep;
}

inline auto unpack(u32 move_int) {
    i8 r = move_int & 0x7;
    i8 c = (move_int >> 3) & 0x7;
    i8 move_type = ((move_int >> 6) & 0x3) - 1;
    return tuple{r, c, move_type};
}

inline u64 pack_tt(u64 uphash, i16 score, u64 depth, u64 flag, u64 best_move, u64 generation) {
    u64 result = 0;
    result |= best_move;
    result |= flag << 16;
    result |= (depth-1)/2 << 18;
    result |= (u64)(score + 131072) << 22;
    result |= generation << 40;
    result |= uphash << 41;
    return result;
}

inline auto unpack_tt(u64 tt_int) {
    u16 best_move = tt_int & 0xffff;
    u8 flag = (tt_int >> 16) & 0x3;
    u8 depth = (tt_int >> 18) & 0xf;
    i16 score = ((i16)((tt_int >> 22) & 0x3ffff) - 131072);
    u32 generation = (tt_int >> 40) & 0x1;
    u32 uphash = (tt_int >> 41) & 0x7fffffff;
    return tuple{uphash, score, depth, flag, best_move, generation};
}

inline u32 pack_tt32(u32 uphash, i8 score, u64 depth, u64 flag, u64 best_move_idx, u64 generation) {
    u32 result = 0;
    result |= best_move_idx;
    result |= flag << 5;
    result |= (depth-1)/2 << 7;
    result |= (u32)(score + 64) << 9;
    result |= generation << 16;
    result |= uphash << 17;
    return result;
}

inline auto unpack_tt32(u32 tt_int) {
    u16 best_move_idx = tt_int & 0x1f;
    u8 flag = (tt_int >> 5) & 0x3;
    u8 depth = (tt_int >> 7) & 0x3;
    i16 score = ((i16)((tt_int >> 9) & 0x7f) - 64);
    u32 generation = (tt_int >> 16) & 0x1;
    u32 uphash = (tt_int >> 17) & 0x7fff;
    return tuple {uphash, score, depth, flag, best_move_idx, generation};
}


inline auto bucket1(u64 hash) {
    u32 ret = 0xFFFFFFFF;
    const u32 target_tag = hash & 0x7fff;
    for (i32 i = 0; i < bmems; i++) {
        u32 entry = TT[hash >> (64 - tts)].entries[i];
        u32 tag = (entry >> 17) & 0x7fff;
        if (tag == target_tag) {
            return entry;
        }
    }
    return ret;
}



auto bucket2(u64 hash, i16 score, u64 depth, u64 flag, u64 best_move_idx, u64 generation){
    auto& B = TT[hash >> (64-tts)];
    i32 min_d = 999999;
    i32 idx = bmems;
    i32 same = bmems;
    i32 genc = bmems;
    u64 hs = hash & 0x7fff;

    for(i32 i=0; i<bmems-1; i++){
        auto [uphash_, d2, depth_, flag_, d3, gen_] = unpack_tt32(B.entries[i]);
        idx = (depth_>min_d) ? i : idx;
        min_d = (depth_>min_d) ? depth_ : min_d;
        genc = (gen_!=generation) ? i : genc; 
        same = (uphash_ == hs) ? i : same; 
    }

    auto write = pack_tt32(hs, score, depth, flag, best_move_idx, generation);

    idx = (genc!=bmems) ? genc : idx;
    idx = (same!=bmems) ? same : idx;
    volatile u32 pull = B.entries[0];
    write = (idx==bmems) ? pull : write;
    idx = (idx==bmems) ? 0 : idx;
    B.entries[idx] = write;
};

