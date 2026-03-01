#!/bin/bash
# Compile C++ logic bridge to shared library

g++ -std=c++20 -O3 -march=native -shared -fPIC \
    logic_bridge.cpp -o _breakthrough.so

python3 main.py