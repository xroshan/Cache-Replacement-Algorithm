#!/bin/bash

for number in {1..99..5}
do
	./cacher $number 200000 ./traces/cb_web.trace >> outs/output"$number".txt &
done

