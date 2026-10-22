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

#!/usr/bin/env python3

import os
import sys
import zipfile
import tarfile
import json
import argparse

# Hardcoded list of library extensions to track
LIB_EXTENSIONS = ('.a', '.jar', '.so')

def get_archive_content(path):
    """
    Extracts metadata for library files within a compressed archive.
    Returns a list of dicts with 'name' and 'size' (uncompressed).
    """
    content = []
    
    # .aar and .zip are both handled by zipfile
    if path.endswith(('.zip', '.aar')):
        with zipfile.ZipFile(path, 'r') as z:
            for info in z.infolist():
                # info.is_dir() is available in Python 3.6+
                if not info.is_dir() and info.filename.endswith(LIB_EXTENSIONS):
                    content.append({
                        "name": info.filename,
                        "size": info.file_size
                    })
                    
    # .tgz or .tar.gz handled by tarfile
    elif path.endswith(('.tgz', '.tar.gz')):
        with tarfile.open(path, 'r:gz') as t:
            for member in t.getmembers():
                if member.isfile() and member.name.endswith(LIB_EXTENSIONS):
                    content.append({
                        "name": member.name,
                        "size": member.size
                    })
    else:
        raise ValueError(f"Unsupported archive format: {path}")
        
    return content

def main():
    parser = argparse.ArgumentParser(description="Dump sizes of library files inside archives as JSON.")
    parser.add_argument('archives', nargs='+', help='List of compressed archives (.aar, .tgz, .zip)')
    args = parser.parse_args()

    results = []
    
    for archive_path in args.archives:
        if not os.path.exists(archive_path):
            print(f"Error: File not found: {archive_path}", file=sys.stderr)
            sys.exit(1)
            
        try:
            archive_full_size = os.path.getsize(archive_path)
            inner_content = get_archive_content(archive_path)
            
            # Sort the content list alphabetically by name
            inner_content.sort(key=lambda x: x['name'])
            
            results.append({
                "name": os.path.basename(archive_path),
                "size": archive_full_size,
                "content": inner_content
            })
        except Exception as e:
            print(f"Error processing {archive_path}: {e}", file=sys.stderr)
            sys.exit(1)

    # Sort the top-level results by archive name
    results.sort(key=lambda x: x['name'])

    # Output JSON to stdout
    print(json.dumps(results, indent=2))

if __name__ == "__main__":
    main()
