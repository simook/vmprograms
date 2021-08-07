#!/usr/bin/env bash
file="$1"
tenant="$2"
host="http://127.0.0.1:8080"
key="12daf155b8508edc4a4b8002264d7494"

source scripts/live_update.sh "$host" "$tenant" "$key" "$file"
