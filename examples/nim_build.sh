#!/usr/bin/env bash
project="${1-hello}"
nim c --app:staticLib --noMain --passL:-static $project.nim
gcc -static -O2 -Wl,--whole-archive lib$project.a -Wl,--no-whole-archive nim/main.c -o $project
rm -f lib$project.a
