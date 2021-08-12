#!/usr/bin/env bash
set -e
mkdir -p build
pushd build
cmake .. -G Ninja
ninja
popd
bash ../../upload.sh build/bin/dynsite wpizza.com
