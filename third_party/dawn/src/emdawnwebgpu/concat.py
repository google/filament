#!/usr/bin/env python3

# Concatenates files.
# Usage: python3 concat.py OUT_FILE IN_FILE [IN_FILE ...]

import sys

out_filename = sys.argv[1]
input_filenames = sys.argv[2:]

with open(out_filename, 'w') as out_file:
    for input_filename in input_filenames:
        with open(input_filename) as input_file:
            for line in input_file:
                out_file.write(line)
