#!/usr/bin/env bash
set -e
file="${1-build/mandelbrot_avx2}"
tenant="zpizza.com"
host="http://127.0.0.1:8080"
key="12daf155b8508edc4a4b8002264d7494"

echo "Sending $file to tenant $tenant at $host"
curl -H "X-PostKey: $key" -H "Host: $tenant" --data-binary "@$file" -X POST $host

N=20
WRK="$HOME/git/wrk/wrk"

warmup=$($WRK -c1 -t1 -d 2s $host/z)

for T in $(seq 1 $N);
do
	output=$($WRK -c ${T} -t ${T} -d 5s $host/z | grep Latency)
	arr=($output)
	average=${arr[1]}
	lowest=${arr[2]}
	highest=${arr[3]}
	echo $T: $average $lowest $highest
done
