#!/usr/bin/env bash
set -e
mkdir -p build
python3 static_builder.py www build/static_site.c
touch main.c
pushd build
ln -fs ../../api.h api.h
ln -fs ../crc32.h  crc32.h
ln -fs ../www  www
cmake .. -G Ninja
ninja -d explain
popd
time source ../../upload.sh build/jsapp jpizza.com
