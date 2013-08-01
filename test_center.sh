#!/usr/sh

TEST="./tests"
for i in `ls $TEST`; do
	if test -f $TEST/$i; then
		`./face $TEST/$i $TEST/fource_center/$i`
	fi
done
