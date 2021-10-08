#!/usr/bin/env bash
set -e

mkdir -p build
pushd build

cmake .. -G Ninja -DMULTIPROCESS=OFF -DNOSIMD=ON -DAVX512F=OFF -DSSE4=OFF
ninja
mv vectorize vectorize_nosimd

cmake .. -G Ninja -DNOSIMD=OFF -DAVX512F=OFF -DSSE4=ON
ninja
mv vectorize vectorize_sse42

cmake .. -G Ninja -DNOSIMD=OFF -DAVX512F=OFF -DSSE4=OFF
ninja
mv vectorize vectorize_avx2

cmake .. -G Ninja -DNOSIMD=OFF -DAVX512F=ON -DSSE4=OFF
ninja
mv vectorize vectorize_avx512

cmake .. -G Ninja -DMULTIPROCESS=ON -DNOSIMD=OFF -DAVX512F=OFF -DSSE4=OFF
ninja
mv vectorize vectorize_mp_avx2

popd
