#!/usr/bin/env bash
tmpfile=$(mktemp /tmp/abc-script.XXXXXX)

source="$1"
tenant="${2-xpizza.com}"

gcc -static -O2 $source -o "$tmpfile"

./upload.sh "$tmpfile" "$tenant"

rm -f "$tmpfile"
