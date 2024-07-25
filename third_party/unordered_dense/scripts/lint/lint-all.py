#!/usr/bin/env python3

from glob import glob
from pathlib import Path
from subprocess import run
from os import path
from time import time


time_start = time()

exit_code = 0
num_linters = 0
mod_path = Path(__file__).parent
for lint in glob(f"{mod_path}/lint-*"):
    lint = path.abspath(lint)
    if lint == path.abspath(__file__):
        continue

    num_linters += 1
    result = run([lint])
    if result.returncode == 0:
        continue

    print(f"^---- failure from {lint.split('/')[-1]}")
    exit_code |= result.returncode

time_end = time()
print(f"{num_linters} linters in {time_end - time_start:0.2}s")
exit(exit_code)
