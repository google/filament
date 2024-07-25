#!/usr/bin/env python3

from glob import glob
from pathlib import Path
from subprocess import run
from os import path
import subprocess
import sys
from time import time
import re

root_path = path.abspath(Path(__file__).parent.parent.parent)

globs = [
    f"{root_path}/include/**/*.h",
    f"{root_path}/test/**/*.h",
    f"{root_path}/test/**/*.cpp",
]
exclusions = [
    "nanobench\\.h",
    "FuzzedDataProvider\\.h",
    '/third-party/']

files = []
for g in globs:
    r = glob(g, recursive=True)
    files.extend(r)

# filter out exclusions
for exclusion in exclusions:
    l = filter(lambda file: re.search(exclusion, file) == None, files)
    files = list(l)

if len(files) == 0:
    print("could not find any files!")
    sys.exit(1)

command = ['clang-format', '--dry-run', '-Werror'] + files
p = subprocess.Popen(command,
                     stdout=subprocess.PIPE,
                     stderr=None,
                     stdin=subprocess.PIPE,
                     universal_newlines=True)

stdout, stderr = p.communicate()

print(f"clang-format checked {len(files)} files")

if p.returncode != 0:
    sys.exit(p.returncode)
