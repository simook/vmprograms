#!/usr/bin/env bash
set -e
source assert.sh
tmpfile=$(mktemp /tmp/abc-script.XXXXXX)
gcc -static -O2 ../examples/hello.c -o "$tmpfile"
host="http://localhost:8080/"
key="12daf155b8508edc4a4b8002264d7494"

function test_tenant() {
	echo $1
	T_UPL=$(curl -s -H "X-PostKey: $key" -H "Host: $1" --data-binary "@$tmpfile" -X POST $host)
	assert_eq "$T_UPL" "Update successful"
	T_GET=$(curl -s $host -H "Host: $1")
	assert_eq "$T_GET" "Hello World!" "GET failed"
}

for j in {0..49}
do
	for i in {1..10}
	do (
		n=$(( j*10+i ))
		test_tenant "tenant$n.com"
	) &
	done
	wait -n
done
