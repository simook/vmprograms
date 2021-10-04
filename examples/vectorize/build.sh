#!/usr/bin/env bash
set -e

mkdir -p build
pushd build

cmake .. -G Ninja -DNOSIMD=ON -DAVX512F=OFF -DSSE4=OFF
ninja
cp vectorize vectorize_nosimd

cmake .. -G Ninja -DNOSIMD=OFF -DAVX512F=OFF -DSSE4=ON
ninja
cp vectorize vectorize_sse42

cmake .. -G Ninja -DNOSIMD=OFF -DAVX512F=OFF -DSSE4=OFF
ninja
cp vectorize vectorize_avx2

cmake .. -G Ninja -DNOSIMD=OFF -DAVX512F=ON -DSSE4=OFF
ninja
cp vectorize vectorize_avx512

popd
