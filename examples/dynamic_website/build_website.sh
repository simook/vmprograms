#!/usr/bin/env bash
set -e
mkdir -p build
pushd build
cmake .. -G Ninja
ninja clean
ninja
popd
bash ../../upload.sh build/dynsite wpizza.com
