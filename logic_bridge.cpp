#include "logic.h"
#include <cstring>
#include <cstdint>
#include <cstdio>

PyMove ai_turn_wrapper() {
    PyMove m = {0, 0, 0, 0};
    // println("{}", sizeof(Bucket));
    ai_turn(10, m.r1, m.c1, m.r2, m.c2);
    return m;
}

extern "C" {
    
    void init_board() {
        memset(b, 0, sizeof(b));
        memset(scores, 0, sizeof(scores));
        
        uint64_t seed = 0x9e3779b97f4a7c15;
        for (int p = 0; p < 3; ++p) {
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    seed ^= seed >> 12;
                    seed ^= seed << 25;
                    seed ^= seed >> 27;
                    const_cast<u64&>(zobra[p][i][j]) = seed * 2685821657736338717ULL;
                }
            }
        }
    }
    
    void set_board(const u8* board_data) {
        memcpy(b, board_data, 64);
    }
    
    void get_board(u8* board_data) {
        memcpy(board_data, b, 64);
    }
    
    void call_ai_turn(u8* out) {
        
        PyMove m = ai_turn_wrapper();
        out[0] = m.r1;
        out[1] = m.c1;
        out[2] = m.r2;
        out[3] = m.c2;
    }
    
    i8 call_gameover() {
        return gameover();
    }
    
    i16 call_state() {
        return state();
    }
    
}
