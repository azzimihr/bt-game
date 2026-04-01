#!/bin/bash

# Optionally build benchmark
if [ "$1" = "bench" ]; then
    clang++ -stdlib=libc++ -std=gnu++23 -O3 -march=native -g -fno-omit-frame-pointer -fdebug-info-for-profiling benchmark.cpp -lc++ -lpthread -o benchmark
    perf record -g ./benchmark
    hotspot perf.data &
else
    clang++ -stdlib=libc++ -std=gnu++23 -O3 -march=native -fno-exceptions -fno-rtti -fno-threadsafe-statics -shared -fPIC logic_bridge.cpp -lc++ -lpthread -o _breakthrough.so
    python3 main.py
fi