#!/usr/bin/env python3
#
# Copyright 2017-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

# nextfree.py - determine the next unused extension numbers.
# Use this when registering a new extension
#
# Use: nextfree.py

import copy, os, re, string, sys

def write(*args, **kwargs):
    file = kwargs.pop('file', sys.stdout)
    end = kwargs.pop('end', '\n')
    file.write(' '.join([str(arg) for arg in args]))
    file.write(end)

# Load the registry
file = 'registry.py'
exec(open(file).read())

# Track each number separately
keys = { 'arbnumber', 'number', 'esnumber', 'scnumber' }
max = {}
for k in keys:
    max[k] = 0

# Loop over all extensions updating the max value
for name,v in registry.items():
    for k in keys:
        if k in v.keys():
            n = v[k]
            if (n > max[k]):
                max[k] = n

# Report next free values
for k in keys:
    write('Next free', k, '=', max[k] + 1)
