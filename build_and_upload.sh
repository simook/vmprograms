#!/usr/bin/env bash
tmpfile=$(mktemp /tmp/abc-script.XXXXXX)

gcc -static -O2 $2 -o "$tmpfile"

tenant="$1"
host="http://141.0.231.216:8080"
key="12daf155b8508edc4a4b8002264d7494"

source scripts/live_update.sh "$host" "$tenant" "$key" "$tmpfile"

rm -f "$tmpfile"
