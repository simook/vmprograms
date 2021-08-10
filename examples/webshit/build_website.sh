#!/usr/bin/env bash
set -e
python static_builder.py
g++ -std=c++17 -static -O2 -march=native -I. /tmp/static_builder.cpp -o /tmp/vpizza
bash ../../upload.sh /tmp/vpizza vpizza.com
