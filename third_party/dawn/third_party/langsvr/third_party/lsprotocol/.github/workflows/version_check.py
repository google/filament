# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.


import datetime
import pathlib
import sys

pyproject = (
    pathlib.Path(__file__).parent.parent.parent
    / "packages"
    / "python"
    / "pyproject.toml"
)

content = pyproject.read_text(encoding="utf-8")

for line in content.splitlines():
    if line.startswith("version"):
        version = line.split("=")[1].strip().strip('"')
        year, minor, micro = version.split(".")
        today = datetime.date.today()
        if int(year) not in [today.year, today.year + 1]:
            print(f"Version {version} year should be {today.year} or {today.year + 1}")
            sys.exit(1)

print("Version check passed")
