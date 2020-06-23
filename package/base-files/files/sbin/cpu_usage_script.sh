#!/bin/ash

cpudatatick=$(top -n1 | awk ' { print $7 }' | tail -n20 | sed 's/[^0-9]//g' | awk '{ sum+=$1} END {print sum}')
echo "$cpudatatick" >> "/tmp/top_output"

cpudataloglength=$(wc -w < /tmp/top_output)

if [ $cpudataloglength -ge 4 ]
then
cpufivedata=$(cat /tmp/top_output | tail -n4)
cpudataloglength=4
echo "$cpufivedata" > "/tmp/top_output"
fi

cpudatasum=0; for i in `cat /tmp/top_output`; do cpudatasum=$(($cpudatasum + $i)); done;
cpuloadaverage=$(awk "BEGIN {printf \"%.2f\",${cpudatasum}/${cpudataloglength}}")

echo "$cpuloadaverage"




