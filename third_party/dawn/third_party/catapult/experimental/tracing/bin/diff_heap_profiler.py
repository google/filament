#!/usr/bin/env vpython3
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import print_function
import argparse
import gzip
import json
import os
import shutil
import six
from six.moves import zip

_OUTPUT_DIR = 'output'
_OUTPUT_GRAPH_DIR = os.path.join(_OUTPUT_DIR, 'graph')


class Process(object):

  def __init__(self):
    self.pid = None
    self.name = None
    self.labels = None
    self.types = {}
    self.strings = {}
    self.stackframes = {}
    self.allocators = None
    self.version = None


class Entry(object):

  def __init__(self):
    self.count = None
    self.size = None
    self.type = None
    self.stackframe = None


class GraphDump(object):

  def __init__(self):
    self.pid = None
    self.name = None
    self.labels = None
    self.heap = None
    self.root = ''
    self.leaks = ''
    self.leak_stackframes = 0
    self.leak_objects = 0

def OpenTraceFile(file_path, mode):
  if file_path.endswith('.gz'):
    return gzip.open(file_path, mode + 'b')
  return open(file_path, mode + 't')

def FindMemoryDumps(filename):
  processes = {}

  with OpenTraceFile(filename, 'r') as f:
    data = json.loads(f.read())

    for event in data['traceEvents']:
      pid = event['pid']
      if pid not in processes:
        processes[pid] = Process()
        processes[pid].pid = pid
      process = processes[pid]

      # Retrieve process informations.
      if event['ph'] == 'M':
        if event['name'] == 'process_name' and 'name' in event['args']:
          process.name = event['args']['name']
        if event['name'] == 'process_labels' and 'labels' in event['args']:
          process.labels = event['args']['labels']

      if event['name'] == 'typeNames':
        process.types = {}
        for type_id, t in six.iteritems(event['args']['typeNames']):
          process.types[int(type_id)] = t

      if event['name'] == 'stackFrames':
        process.stackframes = {}
        for stack_id, s in six.iteritems(event['args']['stackFrames']):
          new_stackframe = {}
          new_stackframe['name'] = s['name']
          if 'parent' in s:
            new_stackframe['parent'] = int(s['parent'])
          process.stackframes[int(stack_id)] = new_stackframe

      # Look for a detailed memory dump event.
      if not ((event['name'] == 'periodic_interval' or
               event['name'] == 'explicitly_triggered') and
              event['args']['dumps']['level_of_detail'] == 'detailed'):
        continue

      # Check for a memory dump V1.
      if u'heaps' in event['args']['dumps']:
        # Get the first memory dump.
        if not process.allocators:
          process.version = 1
          process.allocators = event['args']['dumps']['heaps']

      # Check for a memory dump V2.
      # See format: [chromium] src/base/trace_event/heap_profiler_event_writer.h
      if u'heaps_v2' in event['args']['dumps']:
        # Memory dump format V2 is dumping information incrementally. Update
        # the cumulated indexes.
        maps = event['args']['dumps']['heaps_v2']['maps']
        for string in maps['strings']:
          process.strings[string['id']] = string['string']

        for node in maps['nodes']:
          node_v1 = {}
          node_v1['name'] = process.strings[node['name_sid']]
          if 'parent' in node:
            node_v1['parent'] = node['parent']
          process.stackframes[node['id']] = node_v1

        for t in maps['types']:
          process.types[t['id']] = process.strings[t['name_sid']]

        # Get the first memory dump.
        if not process.allocators:
          dump = event['args']['dumps']
          process.version = 2
          process.allocators = dump['heaps_v2']['allocators']

  # Remove processes with incomplete memory dump.
  # Note: Calling list() otherwise we can't modify list while iterating.
  for pid, process in list(processes.items()):
    if not (process.allocators and process.stackframes and process.types):
      del processes[pid]

  return processes


def ResolveMemoryDumpFields(entries, stackframes, types):
  def ResolveStackTrace(stack_id, stackframes):
    stackframe = stackframes[stack_id]
    tail = ()
    if 'parent' in stackframe:
      tail = ResolveStackTrace(stackframe['parent'], stackframes)
    name = stackframe['name'].replace('\r', '').replace('\n', '')
    return (name,) + tail

  def ResolveType(type_id, types):
    return types[type_id]

  for entry in entries:
    # Stackframe may be -1 (18446744073709551615L) when not stackframe are
    # available.
    if entry.stackframe not in stackframes:
      entry.stackframe = []
    else:
      entry.stackframe = ResolveStackTrace(entry.stackframe, stackframes)
    entry.type = ResolveType(entry.type, types)


def IncrementHeapEntry(stack, count, size, typename, root):
  if not stack:
    root['count'] += count
    root['size'] += size
    if typename not in root['count_by_type']:
      root['count_by_type'][typename] = 0
    root['count_by_type'][typename] += count
  else:
    top = stack[-1]
    tail = stack[:-1]

    if top not in root['children']:
      new_node = {}
      new_node['count'] = 0
      new_node['size'] = 0
      new_node['children'] = {}
      new_node['count_by_type'] = {}
      root['children'][top] = new_node

    IncrementHeapEntry(tail, count, size, typename, root['children'][top])


def CanonicalHeapEntries(root):
  total_count = 0
  total_size = 0
  for child in six.itervalues(root['children']):
    total_count += child['count']
    total_size += child['size']
  root['count'] -= total_count
  root['size'] -= total_size

  for typename in root['count_by_type']:
    total_count_for_type = 0
    for child in six.itervalues(root['children']):
      if typename in child['count_by_type']:
        total_count_for_type += child['count_by_type'][typename]
    root['count_by_type'][typename] -= total_count_for_type

  for child in six.itervalues(root['children']):
    CanonicalHeapEntries(child)


def FindLeaks(root, stack, leaks, threshold, size_threshold):
  for frame in root['children']:
    FindLeaks(root['children'][frame], [frame] + stack, leaks, threshold,
              size_threshold)

  if root['count'] > threshold and root['size'] > size_threshold:
    leaks.append({'count': root['count'],
                  'size': root['size'],
                  'count_by_type': root['count_by_type'],
                  'stackframes': stack})

def DumpTree(root, frame, output, threshold, size_threshold):
  output.write('\n{ \"name\": \"%s\",' % frame)
  if root['count'] > threshold and root['count'] > size_threshold:
    output.write(' \"size\": \"%s\",' % root['size'])
    output.write(' \"count\": \"%s\",' % root['count'])
  output.write(' \"children\": [')
  is_first = True
  for child_frame, child in root['children'].items():
    if is_first:
      is_first = False
    else:
      output.write(',')

    DumpTree(child, child_frame, output, threshold, size_threshold)
  output.write(']')
  output.write('}')


def GetEntries(heap, process):
  """
  Returns all entries in a heap, after filtering out unknown entries, and doing
  some post processing to extract the relevant fields.
  """
  if not process:
    return []

  entries = []
  if process.version == 1:
    for raw_entry in process.allocators[heap]['entries']:
      # Cumulative sizes and types are skipped. see:
      # https://chromium.googlesource.com/chromium/src/+/a990af190304be5bf38b120799c594df5a293518/base/trace_event/heap_profiler_heap_dump_writer.cc#294
      if 'type' not in raw_entry or not raw_entry['bt']:
        continue

      entry = Entry()
      entry.count = int(raw_entry['count'], 16)
      entry.size = int(raw_entry['size'], 16)
      entry.type = int(raw_entry['type'])
      entry.stackframe = int(raw_entry['bt'])
      entries.append(entry)

  elif process.version == 2:
    raw_entries = list(zip(process.allocators[heap]['counts'],
                      process.allocators[heap]['sizes'],
                      process.allocators[heap]['types'],
                      process.allocators[heap]['nodes']))
    for (raw_count, raw_size, raw_type, raw_stackframe) in raw_entries:
      entry = Entry()
      entry.count = raw_count
      entry.size = raw_size
      entry.type = raw_type
      entry.stackframe = raw_stackframe
      entries.append(entry)

  # Resolve fields by looking into indexes
  ResolveMemoryDumpFields(entries, process.stackframes, process.types)

  return entries


def FilterProcesses(processes, filter_by_name, filter_by_labels):
  remaining_processes = {}
  for pid, process in six.iteritems(processes):
    if filter_by_name and process.name != filter_by_name:
      continue
    if (filter_by_labels and
        (not process.labels or filter_by_labels not in process.labels)):
      continue
    remaining_processes[pid] = process

  return remaining_processes


def FindRelevantProcesses(start_trace, end_trace,
                          filter_by_name,
                          filter_by_labels,
                          match_by_labels):
  # Retrieve the processes and the associated memory dump.
  end_processes = FindMemoryDumps(end_trace)
  end_processes = FilterProcesses(end_processes, filter_by_name,
                                  filter_by_labels)

  start_processes = None
  if start_trace:
    start_processes = FindMemoryDumps(start_trace)
    start_processes = FilterProcesses(start_processes, filter_by_name,
                                      filter_by_labels)

  # Build a sequence of pair of processes to be compared.
  processes = []
  if not start_processes:
    # Only keep end-processes.
    for _, end_process in six.iteritems(end_processes):
      processes.append((None, end_process))
  elif match_by_labels:
    # Processes are paired based on name/labels.
    for _, end_process in six.iteritems(end_processes):
      matching_start_process = None
      for _, start_process in six.iteritems(start_processes):
        if (start_process.name == end_process.name and
            (start_process.name in ['Browser', 'GPU'] or
             start_process.labels == end_process.labels)):
          matching_start_process = start_process

      if matching_start_process:
        processes.append((matching_start_process, end_process))
  else:
    # Processes are paired based on their PID.
    relevant_pids = set(end_processes.keys()) & set(start_processes.keys())
    for pid in relevant_pids:
      start_process = start_processes[pid]
      end_process = end_processes[pid]
      processes.append((start_process, end_process))

  return processes


def BuildGraphDumps(processes, threshold, size_threshold):
  """
  Build graph for a sequence of pair of processes.
  If start_process is None, counts objects in end_trace.
  Otherwise, counts objects present in end_trace, but not in start_process.
  """

  graph_dumps = []

  for (start_process, end_process) in processes:
    pid = end_process.pid
    name = end_process.name if end_process.name else ''
    labels = end_process.labels if end_process.labels else ''
    print('Process[%d] %s: %s' % (pid, name, labels))

    for heap in end_process.allocators:
      start_entries = GetEntries(heap, start_process)
      end_entries = GetEntries(heap, end_process)

      graph = GraphDump()
      graph.pid = pid
      graph.name = name
      graph.labels = labels
      graph.heap = heap
      graph_dumps.append(graph)

      # Do the math: diffing start and end memory dumps.
      root = {}
      root['count'] = 0
      root['size'] = 0
      root['children'] = {}
      root['count_by_type'] = {}

      for entry in start_entries:
        if entry.type:
          IncrementHeapEntry(entry.stackframe, - entry.count, - entry.size,
                             entry.type, root)
      for entry in end_entries:
        if entry.type:
          IncrementHeapEntry(entry.stackframe, entry.count, entry.size,
                             entry.type, root)

      CanonicalHeapEntries(root)

      graph.root = root

      # Find leaks
      leaks = []
      FindLeaks(root, [], leaks, threshold, size_threshold)
      leaks.sort(reverse=True, key=lambda k: k['size'])

      if leaks:
        print('  %s: %d potential leaks found.' % (heap, len(leaks)))
        graph.leaks = leaks
        graph.leak_stackframes = len(leaks)
        for leak in leaks:
          graph.leak_objects += leak['count']

  return graph_dumps


def WritePotentialLeaks(graph_dumps):
  for graph in graph_dumps:
    if graph.leaks:
      filename = 'process_%d_%s-leaks.json' % (graph.pid, graph.heap)
      output_filename = os.path.join(_OUTPUT_DIR, filename)
      with open(output_filename, 'w') as output:
        json.dump(graph.leaks, output)


def WriteGrahDumps(graph_dumps, threshold, size_threshold):
  for graph in graph_dumps:
    # Dump the remaining allocated objects tree.
    filename = 'process_%d_%s-objects.json' % (graph.pid, graph.heap)
    output_filename = os.path.join(_OUTPUT_GRAPH_DIR, filename)
    if graph.root:
      with open(output_filename, 'w') as output:
        DumpTree(graph.root, '.', output, threshold, size_threshold)
      graph.root = filename


def WriteIndex(graph_dumps):
  output_filename = os.path.join(_OUTPUT_GRAPH_DIR, 'index.json')
  with open(output_filename, 'w') as output:
    json.dump([
        {'pid': graph.pid,
         'heap': graph.heap,
         'name': graph.name,
         'labels': graph.labels,
         'objects': graph.root,
         'potential leaks': graph.leak_stackframes,
         'objects leaked': graph.leak_objects,
        }
        for graph in graph_dumps], output)


def WriteHTML():
  # Copy the HTML page.
  source = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        'diff_heap_profiler.html')
  destination = os.path.join(_OUTPUT_GRAPH_DIR, 'index.html')
  shutil.copyfile(source, destination)

  # Copy the D3 library file.
  source = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        os.path.pardir,
                        os.path.pardir,
                        os.path.pardir,
                        'tracing',
                        'third_party',
                        'd3',
                        'd3.min.js')
  destination = os.path.join(_OUTPUT_GRAPH_DIR, 'd3.min.js')
  shutil.copyfile(source, destination)


def Main():
  parser = argparse.ArgumentParser()
  parser.add_argument(
      '--flame-graph',
      action='store_true',
      help='Output a flame graph based on stackframe allocations')
  parser.add_argument(
      '--threshold',
      type=int,
      default=0,
      help='Objects threshold for being a potential memory leak')
  parser.add_argument(
      '--size-threshold',
      type=int,
      default=0,
      help='Size threshold for being a potential memory leak')
  parser.add_argument(
      '--filter-by-name',
      type=str,
      help='Only keep processes with name (i.e. Browser, Renderer, ...)')
  parser.add_argument(
      '--filter-by-labels',
      type=str,
      help='Only keep processes with matching labels')
  parser.add_argument(
      '--match-by-labels',
      action='store_true',
      help='Match processes between runs by labels')
  parser.add_argument(
      'trace',
      nargs='+',
      help='Trace files to be processed')
  options = parser.parse_args()

  if options.threshold == 0 and options.size_threshold == 0:
    options.threshold = 1000

  if len(options.trace) == 1:
    end_trace = options.trace[0]
    start_trace = None
  else:
    start_trace = options.trace[0]
    end_trace = options.trace[1]

  if not os.path.exists(_OUTPUT_DIR):
    os.makedirs(_OUTPUT_DIR)

  # Find relevant processes to be processed.
  processes = FindRelevantProcesses(start_trace, end_trace,
                                    options.filter_by_name,
                                    options.filter_by_labels,
                                    options.match_by_labels)

  graph_dumps = BuildGraphDumps(processes, options.threshold,
                                options.size_threshold)

  WritePotentialLeaks(graph_dumps)

  if options.flame_graph:
    if not os.path.exists(_OUTPUT_GRAPH_DIR):
      os.makedirs(_OUTPUT_GRAPH_DIR)
    WriteGrahDumps(graph_dumps, options.threshold, options.size_threshold)
    WriteIndex(graph_dumps)
    WriteHTML()

if __name__ == '__main__':
  Main()
