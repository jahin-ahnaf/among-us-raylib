#!/usr/bin/env bash
set -euo pipefail

g++ -std=c++17 main.cpp enet.cpp $(pkg-config --cflags --libs raylib) -lenet -o main
