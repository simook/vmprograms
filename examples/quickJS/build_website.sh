#!/usr/bin/env bash
set -e
mkdir -p build
python3 static_builder.py www build/static_site.c
pushd build
cmake .. -G Ninja
ninja
popd
time source ../../upload.sh build/jsapp jpizza.com
