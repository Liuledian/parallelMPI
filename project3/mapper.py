#!/usr/bin/python
import sys

for line in sys.stdin:
    words = line.strip().split()
    print(words[1][5:7]+'\t'+words[3]+'\t'+words[3])
sys.exit(0)
