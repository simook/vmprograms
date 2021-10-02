#!/usr/bin/env bash
set -e

mkdir -p build
pushd build

cmake .. -G Ninja -DAVX512F=OFF -DSSE4=ON
ninja
cp mandelbrot mandelbrot_sse42

cmake .. -G Ninja -DAVX512F=OFF -DSSE4=OFF
ninja
cp mandelbrot mandelbrot_avx2

cmake .. -G Ninja -DAVX512F=ON -DSSE4=OFF
ninja
cp mandelbrot mandelbrot_avx512

popd
