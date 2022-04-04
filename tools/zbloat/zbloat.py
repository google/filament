#!/usr/bin/env python3
#
# Copyright 2020 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
For usage help, see README.md
"""

import argparse
import os
import struct
import tempfile
import zipfile
import json

from pathlib import Path

TEMPDIR = tempfile.gettempdir()
SCRIPTDIR = Path(os.path.realpath(__file__)).parent
DEBUG = False
RESGEN_MAGIC = b'__RESGEN__'

def format_bytes(bytes, prec=1):
    """Pretty-print a number of bytes."""
    if bytes > 1e6:
        bytes = bytes / 1.0e6
        return f'%.{prec}fm' % bytes
    if bytes > 1e3:
        bytes = bytes / 1.0e3
        return f'%.{prec}fk' % bytes
    return str(bytes)


def get_hint_marker(filename):
    HINTS = ['renderer', 'libfilament-jni']
    hints = [hint in filename for hint in HINTS]
    return '*' if True in hints else ' '


def choose_from_dir(path: Path):
    paths = list(path.glob('**/*.so'))
    has_dso = False
    for index, dso in enumerate(paths):
        has_dso = True
        size = dso.stat().st_size / 1000
        filename = str(dso)
        marker = get_hint_marker(filename)
        print(f"{index:3} {marker} {size:8.0f} kB {filename}")
    has_dso or quit("No .so file found in the specified path")
    val = input("Which dso should be analyzed? Type a number. ")
    return str(paths[int(val)])


def choose_from_zip(path: Path):
    z = zipfile.ZipFile(path)
    paths = []
    for info in z.infolist():
        if info.filename.endswith('.so'):
            paths.append((info.filename, info.compress_size, info.file_size))
    has_dso = False
    for index, dso in enumerate(paths):
        has_dso = True
        filename, csize, usize = dso
        csize = csize / 1000
        usize = usize / 1000
        marker = get_hint_marker(filename)
        print(f"{index:3} {marker} {usize:8.0f} kB ({csize:8.0f} kB) {filename}")
    has_dso or quit("No .so file found in the specified path")
    val = input("Which dso should be analyzed? Type a number. ")
    chosen_filename = paths[int(val)][0]
    result_path = z.extract(chosen_filename, TEMPDIR)
    if DEBUG: os.system(f'cp {result_path} .')
    return result_path


def extract_single_blob(content, index, jsons, blobs):
    offset = content.find(RESGEN_MAGIC, index)
    if offset < 0:
        return 0
    idx = offset + len(RESGEN_MAGIC)
    if chr(content[idx]) == '_':
        return idx
    size_string = ""
    while chr(content[idx]) != '{':
        size_string += chr(content[idx])
        idx += 1
        if len(size_string) >= 10: quit('Bad JSON chunk: ' + size_string)
    json_string = content[idx:idx+int(size_string)]
    if DEBUG: print(json_string)
    resgen_json = json.loads(json_string)
    total_size = 0
    for mat in resgen_json:
        total_size += resgen_json[mat]
    resgen_blob = content[offset - total_size:offset]
    jsons.append(resgen_json)
    blobs.append(resgen_blob)
    return idx+int(size_string)


def extract_resgen(path: Path):
    with open(path, mode='rb') as file:
        content = file.read()
    index, jsons, blobs = 0, [], []
    while True:
        index = extract_single_blob(content, index, jsons, blobs)
        if index == 0:
            return jsons, blobs


def get_compressed_size(blob):
    tmpzip = Path(f"{TEMPDIR}/tmp.zip")
    with zipfile.ZipFile(tmpzip, "w") as tmp:
        tmp.writestr("tmp", blob, zipfile.ZIP_DEFLATED)
    return tmpzip.stat().st_size


def treeify_single_blob(resgen_json, resgen_blob):
    csize = format_bytes(get_compressed_size(resgen_blob))
    result = {}
    total_size = 0
    materials = []
    offset = 0
    for mat in resgen_json:
        material_size = resgen_json[mat]
        total_size += material_size

        material_blob = resgen_blob[offset:offset + material_size]
        offset += material_size

        chunk_offset = 0
        chunks = []
        while chunk_offset < len(material_blob):
            chunk_name = material_blob[chunk_offset:chunk_offset + 8][::-1].decode("utf-8")
            chunk_size = struct.unpack_from('i', material_blob[chunk_offset + 8:chunk_offset + 12])[0]
            chunks.append({
                "data": {"$area": chunk_size},
                "name": chunk_name + " " + format_bytes(chunk_size),
                "detail": chunk_name + " " + format_bytes(chunk_size, 3),
            })
            chunk_offset += 12 + chunk_size

        materials.append({
            "data": {"$area": material_size, "$symbol": "read-only data"},
            "name": mat + " " + format_bytes(material_size),
            "children": chunks,
        })

    result["data"] = {"$area": total_size}
    result["name"] = f"materials {format_bytes(total_size)} ({csize})"
    result["children"] = materials
    return result


def treeify_resgen(resgen_jsons, resgen_blobs):
    result = []
    for resgen_json, resgen_blob in zip(resgen_jsons, resgen_blobs):
        result.append(treeify_single_blob(resgen_json, resgen_blob))
    return result


def main(args):
    path = Path(args.input)
    path.exists() or quit("No file or folder at the specified path.")
    if path.is_dir():
        path = choose_from_dir(path)
    elif path.suffix in ['.zip', '.aar', '.apk']:
        path = choose_from_zip(path)

    dsopath = Path(path)
    size = format_bytes(dsopath.stat().st_size, 2)
    csize = format_bytes(get_compressed_size(open(dsopath, 'rb').read()), 2)
    info = f'{size} ({csize})'

    print('Scanning for resgen resources...')
    resgen_jsons, resgen_blobs = extract_resgen(dsopath)

    print('Running nm... (this might take a while)')
    os.system(f'nm -C -S {path} > {TEMPDIR}/nm.out')
    if DEBUG: os.system(f'cp {TEMPDIR}/nm.out .')

    print('Running objdump...')
    os.system(f'objdump -h {path} > {TEMPDIR}/objdump.out')
    if DEBUG: os.system(f'cp {TEMPDIR}/objdump.out .')

    print('Generating treemap JSON...')
    os.system(f'cd {TEMPDIR} ; python3 {SCRIPTDIR}/evmar_bloat.py syms > syms.json')
    os.system(f'cd {TEMPDIR} ; python3 {SCRIPTDIR}/evmar_bloat.py sections > sections.json')

    # Splice the materials JSON into the sections JSON.
    sections_json = json.loads(open(f'{TEMPDIR}/sections.json').read())
    trimmed_json = sections_json
    for child in sections_json['children']:
        if child["name"].startswith('sections '):
            trimmed_json = child
            for section in trimmed_json['children']:
                if section["name"].startswith('rodata'):
                    section["children"] = treeify_resgen(resgen_jsons, resgen_blobs)
    sections_json = "const kSections = " + json.dumps(trimmed_json) + ";\n"

    symbols_json = open(f'{TEMPDIR}/syms.json').read()
    symbols_json = "const kSymbols = " + symbols_json + ";\n"

    if DEBUG: open('syms.json', 'wt').write(symbols_json)
    if DEBUG: open('sections.json', 'wt').write(sections_json)

    print(f'Generating {args.output}...')
    template = open(f'{SCRIPTDIR}/template.html').read()
    template = template.replace('$TITLE$', dsopath.name)
    template = template.replace('$INFO$', info)
    template = template.replace('$SYMBOLS_JSON$', symbols_json)
    template = template.replace('$SECTIONS_JSON$', sections_json)
    open(args.output, 'w').write(template)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="index.html", help="generated HTML file name")
    parser.add_argument("input", help="path to folder, zip, aar, apk, or so")
    args = parser.parse_args()
    main(args)
