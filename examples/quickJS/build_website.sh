#!/usr/bin/env bash
set -e
mkdir -p build
python static_builder.py www build/static_site.c
pushd build
ln -fs ../../api.h api.h
ln -fs ../crc32.h  crc32.h
ln -fs ../www  www
cmake .. -G Ninja
ninja -d explain
popd
bash ../../upload.sh build/jsapp jpizza.com
