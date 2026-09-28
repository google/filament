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

"""Turns a macOS .ips crash report into a readable backtrace.

macOS writes no core for our samples, but ReportCrash leaves a .ips file with a symbolicated stack
for every thread. That file is two concatenated JSON documents and stores frames as offsets into a
separate image table, so it reads poorly as-is.
"""

import json
import sys


def load_report(path: str) -> tuple[dict, dict]:
    """Returns the (header, body) pair of JSON documents that make up a .ips file."""
    with open(path, 'r') as f:
        header_line = f.readline()
        body = f.read()
    return json.loads(header_line), json.loads(body)


def format_frame(frame: dict, images: list) -> str:
    image_name = '???'
    index = frame.get('imageIndex')
    if index is not None and 0 <= index < len(images):
        image_name = images[index].get('name') or images[index].get('path') or '???'

    symbol = frame.get('symbol')
    if symbol is None:
        # An unsymbolicated frame still locates the code, as an offset into a named image.
        location = f"{image_name} + {frame.get('imageOffset', 0)}"
    else:
        location = f"{symbol} + {frame.get('symbolLocation', 0)} ({image_name})"

    source = ''
    if frame.get('sourceFile'):
        source = f" at {frame['sourceFile']}:{frame.get('sourceLine', 0)}"
    return f'{location}{source}'


def format_report(header: dict, body: dict) -> str:
    lines = []
    lines.append(f"process: {header.get('app_name', '?')}")
    lines.append(f"time:    {header.get('timestamp', '?')}")
    lines.append(f"os:      {header.get('os_version', '?')}")

    exception = body.get('exception', {})
    if exception:
        lines.append(f"exception: {exception.get('type', '?')} "
                     f"signal={exception.get('signal', '?')}")
    if body.get('termination'):
        termination = body['termination']
        lines.append('termination: '
                     f"{termination.get('indicator') or termination.get('reason', '')}")
    if body.get('asi'):
        # Where an assertion message ends up.
        for library, messages in body['asi'].items():
            for message in messages:
                lines.append(f'assertion ({library}): {message}')

    images = body.get('usedImages', [])
    faulting = body.get('faultingThread')
    threads = body.get('threads', [])

    # Faulting thread first: a crash report from a software rasterizer carries several dozen
    # threads, and the one that matters should not have to be hunted for.
    order = list(range(len(threads)))
    if isinstance(faulting, int) and 0 <= faulting < len(threads):
        order.remove(faulting)
        order.insert(0, faulting)

    for i in order:
        thread = threads[i]
        name = thread.get('name') or thread.get('queue') or ''
        marker = ' (faulting)' if i == faulting else ''
        lines.append('')
        lines.append(f'thread {i}{marker} {name}'.rstrip())
        for depth, frame in enumerate(thread.get('frames', [])):
            lines.append(f'  #{depth:<3} {format_frame(frame, images)}')

    return '\n'.join(lines)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'usage: {sys.argv[0]} <report.ips>', file=sys.stderr)
        sys.exit(2)

    try:
        print(format_report(*load_report(sys.argv[1])))
    except Exception as e:
        # A diagnostic that cannot be parsed should not look like a diagnostic that is empty.
        print(f'Could not parse {sys.argv[1]}: {e}', file=sys.stderr)
        sys.exit(1)
