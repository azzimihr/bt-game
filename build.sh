#!/bin/bash
# Compile C++ logic bridge to shared library

clang++ -stdlib=libc++ -std=gnu++23 -O3 -fno-exceptions -fno-rtti -fno-threadsafe-statics -shared -fPIC logic_bridge.cpp -lc++ -o _breakthrough.so

# Optionally build benchmark: ./build.sh bench
if [ "$1" = "bench" ]; then
    clang++ -stdlib=libc++ -std=gnu++23 -O3 -march=native -g -fno-omit-frame-pointer -fdebug-info-for-profiling benchmark.cpp -lc++ -o benchmark
    perf record -g ./benchmark
    hotspot perf.data &
else
    python3 main.py
fi