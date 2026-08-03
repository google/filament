#!/usr/bin/env python3
#
# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import shutil

CUR_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(CUR_DIR, "../../android/filament-android/build/dokka/gfm")
DESTINATION_DIR = os.path.join(CUR_DIR, "../src_mdbook/book/android/dokka")

def run():
    """
    Copies the Dokka-generated GFM (Markdown) documentation files from the build output
    into the mdbook output directory.
    """
    if not os.path.exists(OUT_DIR):
        print(f"Warning: Dokka output directory does not exist: {OUT_DIR}")
        return

    print(f"Copying Dokka documentation from {OUT_DIR} to {DESTINATION_DIR}...")
    if os.path.exists(DESTINATION_DIR):
        shutil.rmtree(DESTINATION_DIR)

    shutil.copytree(OUT_DIR, DESTINATION_DIR)

if __name__ == "__main__":
    run()
