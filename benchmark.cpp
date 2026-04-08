#include "logic.h"
#include <chrono>
#include <random>
#include <cstring>
#include <cstdio>

void init_zobra() {
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

void setup_board() {
    memset(b, 0, sizeof(b));
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {
            b[i][j] = 2;
        }
    }
    for (int i = 6; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            b[i][j] = 1;
        }
    }
}

u8 get_valid_moves(u8 turn, u8 d) {
    auto [p1, p2] = bb(b);
    return bMoves(p1, p2, turn, d);
}

void apply_move(u8 r1, u8 c1, u8 r2, u8 c2, u8 piece) {
    b[r1][c1] = 0;
    b[r2][c2] = piece;
}

bool make_random_move(u8 turn) {
    u8 d = 0;
    u8 move_count = get_valid_moves(turn, d);
    
    if (move_count == 0) {
        return false;
    }
    
    auto [p1, p2] = bb(b);
    memcpy(bt, b, 64);

    sorter2(turn, d, 0, move_count, 0, p1, p2);
    
    static mt19937 rng(random_device{}());
    uniform_int_distribution<u8> dist{0, static_cast<u8>(move_count / 2)};
    
    auto [mt, oi, r, c] = alm[d][dist(rng)];
    
    u8 piece = turn ? 2 : 1;
    if (turn == 0) {
        apply_move(r, c, r - 1, c + mt, piece);
    } else {
        apply_move(r, c, r + 1, c + mt, piece);
    }
    
    return true;
}

void play_game(u8 depth) {
    setup_board();
    
    for (int round = 0; round < 16; ++round) {
        if (!make_random_move(0)) {
            return;
        }
        
        i8 go = gameover();
        if (go) return;
        
        u8 r1, c1, r2, c2;
        ai_turn(depth, r1, c1, r2, c2);
        
        if (r1 == 0 && c1 == 0 && r2 == 0 && c2 == 0) {
            return;
        }
        
        apply_move(r1, c1, r2, c2, 2);
        
        go = gameover();
        if (go) return;
    }
}

int main() {
    freopen("/dev/null", "w", stdout);
    
    init_zobra();

    auto start_time = chrono::high_resolution_clock::now();
    
    constexpr u8 depth = 9;
    constexpr int num_games = 20;
    
    fprintf(stderr, "Running benchmark: %d games, %d depth, max 5 rounds per game\n%lu\n", num_games, depth, tt3);
    
    for (int game = 0; game < num_games; ++game) {
        play_game(depth);
        fprintf(stderr, "  Completed game %d/%d\n", game + 1, num_games);
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    
    fprintf(stderr, "\n=== Benchmark Results ===\n");
    fprintf(stderr, "Total time: %lld ms\n", elapsed.count());
    fprintf(stderr, "Time per game: %.2f ms\n", static_cast<double>(elapsed.count()) / num_games);
    fprintf(stderr, "Transposition table overwrites: %lu\n", overwrites);
    fprintf(stderr, "Nodes searched: %lu\n", searched/8/18);
    
    return 0;
}
