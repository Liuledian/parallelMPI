#!/usr/bin/python
import sys

last_month = None
last_max = -100
last_min = 100

for line in sys.stdin:
    month, temp_max, temp_min = line.rstrip().split('\t')
    temp_max = float(temp_max)
    temp_min = float(temp_min)
    if last_month and last_month != month:
        print(last_month + '\t' + str(last_max) + '\t' + str(last_min))
        (last_month, last_max, last_min) = (month, temp_max, temp_min)
    else:
        last_month, last_max, last_min = month, max(last_max, temp_max), min(last_min, temp_min)

if last_month:
    print(last_month + '\t' + str(last_max) + '\t' + str(last_min))

sys.exit(0)

