#!/usr/bin/env bash
g++ -static -O2 -march=native mandelbrot.cpp lodepng/lodepng.cpp -o /tmp/zpizza
