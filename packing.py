import numpy as np
from numba import njit

@njit(inline='always')
def bb(b):
    p1 = np.uint64(0)
    p2 = np.uint64(0)
    for i in range(7, -1, -1):
        for j in range(7, -1, -1):
            p1 = (p1 << 1) | np.uint64(b[i,j] & 1)
            p2 = (p2 << 1) | np.uint64(b[i,j] >> 1)
    return p1, p2

@njit(inline='always')
def pack(r: np.uint8, c: np.uint8, move_type: np.int8) -> np.uint8:
    move_rep = np.uint8(0) 
    move_rep |= r
    move_rep |= c << 3
    move_rep |= (move_type+1) << 6
    return np.uint8(move_rep)

@njit(inline='always')
def unpack(move_int: np.uint8):
    r = move_int & 0x7
    c = (move_int >> 3) & 0x7
    move_type = (move_int >> 6) & 0x3
    return r, c, move_type - 1

@njit(inline='always')
def pack_tt(uphash: np.uint32, score: np.int16, depth: np.uint8, flag: np.uint8, best_move: np.uint8) -> np.uint64:
    result = np.uint64(0)
    result |= best_move
    result |= flag << 16
    result |= depth << 18
    result |= (score + 131072) << 22
    result |= uphash << 40
    return np.uint64(result)

@njit(inline='always')
def unpack_tt(tt_int):
    best_move = np.uint16(tt_int & 0xffff)
    flag = np.uint8((tt_int >> 16) & 0x3)
    depth = np.uint8((tt_int >> 18) & 0xf)
    score = np.int16(((tt_int >> 22) & 0x3ffff) - 131072)
    uphash = np.uint32((tt_int >> 40) & 0xffffffff)
    return uphash, score, depth, flag, best_move

@njit(inline='always')
def pack_tt32(uphash, score, depth, flag, best_move_idx):
    result = np.uint32(0)
    result |= best_move_idx
    result |= flag << 6
    result |= depth << 8
    result |= (score + 256) << 12
    result |= uphash << 20
    return np.uint32(result)

@njit(inline='always')
def unpack_tt32(tt_int):
    best_move_idx = np.uint8(tt_int & 0x3f)
    flag = np.uint8((tt_int >> 6) & 0x3)
    depth = np.uint8((tt_int >> 8) & 0xf)
    score = np.int8(((tt_int >> 12) & 0xff) - 256)
    uphash = np.uint16((tt_int >> 20) & 0xfff)
    return uphash, score, depth, flag, best_move_idx