#!/usr/bin/env bash
set -e

mkdir -p build
pushd build

cmake .. -G Ninja -DNOSIMD=ON -DAVX512F=OFF -DSSE4=OFF
ninja
cp mandelbrot mandelbrot_multisimd

popd
