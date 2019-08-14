#!/bin/bash
echo "Cleaning up"
rm -f Output.csv

echo "Warming up"
i=1
while [ $i -le 10 ]
do
	./Prac2.py > /dev/null
	((i++))
done

i=1
echo "Running Tests"
while [ $i -le 10 ]
do
	./Prac2.py | grep -oP '\d{2}\.\d+' | sed 's/$/, /' >> Output.csv
	((i++))
done
