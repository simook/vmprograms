#!/usr/bin/env bash
set -e

mkdir -p build
pushd build
cmake .. -G Ninja
ninja
popd

cp build/mandelbrot /tmp/zpizza
