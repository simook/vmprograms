#!/usr/bin/env bash
project="${1-hello}"
nim c --app:staticLib --noMain --cincludes:env --path:env $project.nim
gcc -static -O2 -Wl,--whole-archive lib$project.a -Wl,--no-whole-archive env/main.c -o $project
rm -f lib$project.a

source ../../upload.sh hello wpizza.com
