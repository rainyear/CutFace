#!/usr/sh

TEST="./tests"
for i in `ls $TEST`; do
	if test -f $TEST/$i; then
		`./face $TEST/$i $TEST/detect_face/$i`
	fi
done
