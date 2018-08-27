#!/bin/bash
bin/cacher 1 15 data/msr_src2 4 > output/output1.txt
for number in {5..100..5};
do 
bin/cacher $number 15 data/msr_src2 3 > output/output$number.txt &
done
