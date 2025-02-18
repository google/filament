# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import os
import pathlib
import sys

sys.path.append(os.fspath(pathlib.Path(__file__).parent / "packages" / "python"))
sys.path.append(
    os.fspath(pathlib.Path(__file__).parent / "tests" / "python" / "common")
)
