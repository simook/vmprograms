#!/usr/bin/env bash
set -e
mkdir -p build
pushd build
cmake .. -G Ninja
ninja -d explain
popd
bash ../../upload.sh build/jsapp jpizza.com
