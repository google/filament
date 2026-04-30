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
import glob
import re

CUR_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(CUR_DIR, "../../out/cmake-wasm-release/web/examples/examples")
BOOK_DIR = os.path.join(CUR_DIR, "../src_mdbook/book")

def setup_dirs():
    os.makedirs(f"{BOOK_DIR}/web/assets", exist_ok=True)
    os.makedirs(f"{BOOK_DIR}/web/lib", exist_ok=True)

def copy_libs():
    """
    Copies shared WebGL javascript libraries and WebAssembly binaries from the build output
    into a central web/lib directory in the mdbook output. This ensures all tutorials
    and samples use the same built engine versions.
    """
    libs = ['filament.js', 'filament.wasm', 'gl-matrix-min.js', 'gltumble.min.js']
    for l in libs:
        for root, dirs, files in os.walk(OUT_DIR):
            if l in files:
                src = os.path.join(root, l)
                dst = f"{BOOK_DIR}/web/lib/{l}"
                shutil.copy(src, dst)
                break

def copy_assets():
    """
    Finds all required web assets (textures, models, materials, stylesheets) generated
    during the WebGL build and copies them into the web/assets directory.
    It preserves the relative directory structure per sample so that each sample's
    assets are isolated.
    """
    exts = ['.filamat', '.filamesh', '.ktx', '.ktx2', '.png', '.jpg', '.bin', '.glb', '.gltf', '.hdr', '.css', '.js']
    for root, dirs, files in os.walk(OUT_DIR):
        for f in files:
            if any(f.endswith(ext) for ext in exts):
                src = os.path.join(root, f)
                rel_path = os.path.relpath(src, OUT_DIR)
                dst = os.path.join(BOOK_DIR, "web/assets", rel_path)
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                shutil.copy(src, dst)

def fix_paths():
    """
    Parses the generated HTML examples inside the mdbook output to rewrite their resource URLs.
    Since we moved the libraries to `web/lib` and the assets to `web/assets`, the raw HTML
    files (which assume assets are alongside them) need to be patched to point to the correct
    relative paths based on their nesting depth.
    """
    files_to_fix = glob.glob(f"{BOOK_DIR}/samples/web/*.html") + glob.glob(f"{BOOK_DIR}/remote/*.html") + glob.glob(f"{BOOK_DIR}/web/*.html")

    exts = "filamat|filamesh|ktx|ktx2|png|jpg|bin|glb|gltf|hdr|css|js"
    asset_pattern = r'([\'"`])((?:\.\.\/)?(?:[\w\-\/]+\.)(?:' + exts + r'))\1'

    for f in files_to_fix:
        with open(f, 'r') as file:
            text = file.read()

        sample_name = os.path.splitext(os.path.basename(f))[0]

        # Compute the relative depth prefix depending on where the HTML is located
        if "samples/web/" in f:
            depth_prefix = "../../web/"
        else:
            depth_prefix = "../web/"

        # Replace JS libs
        def js_repl(m):
            quote = m.group(1)
            lib = m.group(3)
            return f"{quote}{depth_prefix}lib/{lib}{quote}"

        text = re.sub(r'([\'"`])(\.\.\/)?(filament\.js|gl-matrix-min\.js|gltumble\.min\.js|filament\.wasm)\1', js_repl, text)

        # Replace assets
        def asset_repl(m):
            quote = m.group(1)
            path = m.group(2)
            if "lib/" in path:
                return m.group(0)
            if path.startswith("../"):
                return f"{quote}{depth_prefix}assets/{path[3:]}{quote}"
            else:
                return f"{quote}{depth_prefix}assets/{sample_name}/{path}{quote}"

        text = re.sub(asset_pattern, asset_repl, text)

        # Fix loadResources base path for glTF models so they can find their external bins/pngs
        # By passing a fully qualified absolute URL using window.location.href, we ensure the
        # browser correctly fetches associated chunks from the web/assets directory.
        def fix_load_resources(m):
            prefix = m.group(1)
            args_str = m.group(2).strip()
            args = [a.strip() for a in args_str.split(',')] if args_str else []
            while len(args) < 2:
                args.append('null')
            args.append(f"new URL('{depth_prefix}assets/{sample_name}/', window.location.href).href")
            return f"{prefix}.loadResources({', '.join(args)});"

        text = re.sub(r'(\w+(?:\.\w+)*)\.loadResources\(([^)]*)\);', fix_load_resources, text)

        text = text.replace('window.FILAMENT_ASSET_DIR = "";', f'window.FILAMENT_ASSET_DIR = "{depth_prefix}assets/{sample_name}/";')

        with open(f, 'w') as file:
            file.write(text)

def run():
    """
    Executes the entire web docs post-processing pipeline:
    1. Sets up the destination directories
    2. Copies the Javascript and WebAssembly libraries
    3. Copies the WebGL generated assets
    4. Rewrites the resource paths inside the HTML examples
    """
    setup_dirs()
    copy_libs()
    copy_assets()
    fix_paths()

if __name__ == "__main__":
    run()
