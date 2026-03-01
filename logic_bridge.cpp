#include "logic.h"
#include <cstring>
#include <cstdint>
#include <cstdio>

extern "C" {
    
    void init_board() {
        memset(b, 0, sizeof(b));
        memset(allm, 0, sizeof(allm));
        memset(scores, 0, sizeof(scores));
        
        // Initialize zobra with deterministic pseudo-random values (LCG)
        uint64_t seed = 0x9e3779b97f4a7c15;
        for (int p = 0; p < 3; ++p) {
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
                    const_cast<u64&>(zobra[p][i][j]) = seed;
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
    
    void call_ai_turn(u8 depth, u8* out) {
        
        Move m = ai_turn_wrapper(depth);
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
