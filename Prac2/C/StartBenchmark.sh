#!/bin/bash
echo "Cleaning up"
rm -f Output.csv

echo "Warming up"
i=1
while [ $i -le 300 ]
do
	make run > /dev/null
	((i++))
done

i=1
echo "Running Tests"
while [ $i -le 10 ]
do
	make run | grep -oP '\d+.+d* +ms' | sed 's/ ms/, /g' >> Output.csv
	((i++))
done
