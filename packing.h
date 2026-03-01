#pragma once

#include <cstdint>
#include <tuple>

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

using namespace std;

inline tuple<u64,u64> bb(const u8 b[8][8]) {
    u64 p1 = 0;
    u64 p2 = 0;
    for (int i = 7; i >= 0; --i) {
        for (int j = 7; j >= 0; --j) {
            p1 = (p1 << 1) | (b[i][j] & 1);
            p2 = (p2 << 1) | ((b[i][j] >> 1) & 1);
        }
    }
    return {p1, p2};
}

inline u8 pack(u64 r, u64 c, u64 move_type) {
    u8 move_rep = 0;
    move_rep |= r;
    move_rep |= c << 3;
    move_rep |= (move_type + 1) << 6;
    return move_rep;
}

inline tuple<u8,u8,i8> unpack(u64 move_int) {
    u8 r = move_int & 0x7;
    u8 c = (move_int >> 3) & 0x7;
    i8 move_type = ((move_int >> 6) & 0x3) - 1;
    return {r, c, move_type};
}

inline u64 pack_tt(u64 uphash, i16 score, u64 depth, u64 flag, u64 best_move) {
    u64 result = 0;
    result |= best_move;
    result |= flag << 16;
    result |= depth << 18;
    result |= (u64)(score + 131072) << 22;
    result |= uphash << 40;
    return result;
}

inline tuple<u32,i16,u8,u8,u16> unpack_tt(u64 tt_int) {
    u16 best_move = tt_int & 0xffff;
    u8 flag = (tt_int >> 16) & 0x3;
    u8 depth = (tt_int >> 18) & 0xf;
    i16 score = (i16)(((tt_int >> 22) & 0x3ffff) - 131072);
    u32 uphash = (tt_int >> 40) & 0xffffffff;
    return {uphash, score, depth, flag, best_move};
}

inline u32 pack_tt32(u64 uphash, i8 score, u64 depth, u64 flag, u64 best_move_idx) {
    u32 result = 0;
    result |= best_move_idx;
    result |= flag << 6;
    result |= depth << 8;
    result |= (u32)(score + 256) << 12;
    result |= uphash << 20;
    return result;
}

inline tuple<u16,i8,u8,u8,u8> unpack_tt32(u32 tt_int) {
    u8 best_move_idx = tt_int & 0x3f;
    u8 flag = (tt_int >> 6) & 0x3;
    u8 depth = (tt_int >> 8) & 0xf;
    i8 score = (i8)(((tt_int >> 12) & 0xff) - 256);
    u16 uphash = (tt_int >> 20) & 0xfff;
    return {uphash, score, depth, flag, best_move_idx};
}
