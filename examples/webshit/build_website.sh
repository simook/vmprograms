#!/usr/bin/env bash
set -e
python static_builder.py
gcc -std=gnu11 -static -O2 -march=native -I. /tmp/static_builder.c -o /tmp/vpizza
bash ../../upload.sh /tmp/vpizza vpizza.com
