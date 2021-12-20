#!/usr/bin/env bash
set -e

mkdir -p build
pushd build

cmake .. -G Ninja -DBUILD_SHARED_LIBS=OFF -DMULTIPROCESS=OFF
ninja

popd
