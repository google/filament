# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Filters a big trace keeping only the last memory-infra dumps."""

from __future__ import print_function

from __future__ import absolute_import
import collections
import gzip
import json


def FormatBytes(value):
  units = ['B', 'KiB', 'MiB', 'GiB']
  while abs(value) >= 1024 and len(units) > 1:
    value /= 1024
    units = units.pop(0)
  return '%3.1f %s' % (value, units[0])


def Main(argv):
  if len(argv) < 2:
    print('Usage: %s trace.json[.gz]' % argv[0])
    return 1

  in_path = argv[1]
  if in_path.lower().endswith('.gz'):
    fin = gzip.open(in_path, 'rb')
  else:
    fin = open(in_path, 'r')
  with fin:
    print('Loading trace (can take 1 min on a z620 for a 1GB trace)...')
    trace = json.load(fin)
    print('Done. Read ' + FormatBytes(fin.tell()))

  print('Filtering events')
  phase_count = collections.defaultdict(int)
  out_events = []
  global_dumps = collections.OrderedDict()
  if isinstance(trace, dict):
    in_events = trace.get('traceEvents', [])
  elif isinstance(trace, list) and isinstance(trace[0], dict):
    in_events = trace

  for evt in in_events:
    phase = evt.get('ph', '?')
    phase_count[phase] += 1

    # Drop all diagnostic events for memory-infra debugging.
    if phase not in ('v', 'V') and evt.get('cat', '').endswith('memory-infra'):
      continue

    # pass-through all the other non-memory-infra events
    if phase != 'v':
      out_events.append(evt)
      continue

    # Recreate the global dump groups
    event_id = evt['id']
    global_dumps.setdefault(event_id, [])
    global_dumps[event_id].append(evt)


  print('Detected %d memory-infra global dumps' % len(global_dumps))
  if global_dumps:
    max_procs = max(len(x) for x in global_dumps.values())
    print('Max number of processes seen: %d' % max_procs)

  ndumps = 2
  print('Preserving the last %d memory-infra dumps' % ndumps)
  detailed_dumps = []
  non_detailed_dumps = []
  for global_dump in global_dumps.values():
    try:
      level_of_detail = global_dump[0]['args']['dumps']['level_of_detail']
    except KeyError:
      level_of_detail = None
    if level_of_detail == 'detailed':
      detailed_dumps.append(global_dump)
    else:
      non_detailed_dumps.append(global_dump)

  dumps_to_preserve = detailed_dumps[-ndumps:]
  ndumps -= len(dumps_to_preserve)
  if ndumps:
    dumps_to_preserve += non_detailed_dumps[-ndumps:]

  for global_dump in dumps_to_preserve:
    out_events += global_dump

  print('\nEvents histogram for the original trace (count by phase)')
  print('--------------------------------------------------------')
  for phase, count in sorted(list(phase_count.items()), key=lambda x: x[1]):
    print('%s %d' % (phase, count))

  out_path = in_path.split('.json')[0] + '-filtered.json'
  print('\nWriting filtered trace to ' + out_path, end='')
  with open(out_path, 'w') as fout:
    json.dump({'traceEvents': out_events}, fout)
    num_bytes_written = fout.tell()
  print(' (%s written)' % FormatBytes(num_bytes_written))
  return 0
