#!/usr/bin/env bash
set -e
source assert.sh

function test_vmcommit() {
	POST_A=$(curl -s http://localhost:8080/x -X POST -d "Hello World 1")
	assert_eq "$POST_A" "Hello World 1" "1st POST failed"
	TEST_A=$(curl -s http://localhost:8080/x)
	assert_eq "$TEST_A" "Hello World 1" "1st GET failed"

	POST_B=$(curl -s http://localhost:8080/x -X POST -d "Hello Posted World 2")
	assert_eq "$POST_B" "Hello Posted World 2" "2nd POST failed"
	TEST_A=$(curl -s http://localhost:8080/x)
	assert_eq "$TEST_A" "Hello Posted World 2" "2nd GET failed"
}

for i in {1..20}
do
	test_vmcommit
done
curl -s http://localhost:8080/x -X POST -d "Hello World" -w @tformat.txt
