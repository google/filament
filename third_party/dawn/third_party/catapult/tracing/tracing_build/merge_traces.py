# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import codecs
import collections
import gzip
import io
import itertools
import json
import logging
import os
import sys

# Add tracing/ to the path.
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import six
from tracing_build import html2trace, trace2html


try:
  StringTypes = six.string_types # pylint: disable=invalid-name
except NameError:
  StringTypes = str


GZIP_FILENAME_SUFFIX = '.gz'
HTML_FILENAME_SUFFIX = '.html'


# Relevant trace event phases. See
# https://code.google.com/p/chromium/codesearch#chromium/src/base/trace_event/common/trace_event_common.h.
METADATA_PHASE = 'M'
MEMORY_DUMP_PHASE = 'v'
BEGIN_PHASE = 'B'
END_PHASE = 'E'
CLOCK_SYNC_EVENT_PHASE = 'c'


# Minimum gap between two consecutive merged traces in microseconds.
MIN_TRACE_GAP_IN_US = 1000000


# Rule for matching IDs in an IdMap. For a given level, |match| should be a
# named tuple class where its fields determine the importance of |entry._items|
# for the purposes of matching pairs of IdMap entries.
IdMapLevel = collections.namedtuple('IdMapLevel', ['name', 'match'])


class IdMap(object):
  """Abstract class for merging and mapping IDs from multiple sources."""

  # Sub-classes must provide a tuple of |IdMapLevel| objects.
  LEVELS = NotImplemented

  def __init__(self, depth=0):
    assert 0 <= depth <= len(self.LEVELS)
    self._depth = depth

    if depth > 0:
      # Non-root node.
      self._canonical_id = None
      self._items = collections.defaultdict(set)
      self._sources = set()

    if depth < len(self.LEVELS):
      # Non-leaf node.
      self._entry_map = {}  # (Source, Entry ID) -> Entry.

  @property
  def max_mapped_id(self):
    """The maximum mapped ID of this map's entries."""
    if not self._entry_map:
      return 0
    return max(e._canonical_id for e in self._entry_map.values())

  def AddEntry(self, source, path, **items):
    """Add a source-specific entry path to the map.

    Args:
      source: The source of the entry (e.g. trace filename).
      path: A path of source-specific entry IDs in the map (e.g. [PID, TID]).
      **items: Dictionary of items (or sets of items) to be appended to the
          target entry's |_items|.
    """
    if path:
      return self._GetSubMapEntry(source, path[0]).AddEntry(source, path[1:],
                                                            **items)
    assert 'id' not in items  # ID is set according to the path.
    for key, value in items.items():
      value_set = self._items[key]
      if (isinstance(value, collections.abc.Iterable) and
          not isinstance(value, StringTypes)):
        value_set.update(value)
      else:
        value_set.add(value)
    return None

  def MapEntry(self, source, path):
    """Map an source-specific entry ID path to a canonical entry ID path.

    Args:
      source: The source of the entry (e.g. trace filename).
      path: A path of source-specific entry IDs in the map (e.g. [PID, TID]).

    Returns:
      A path of canonical entry IDs in the map to which the provided path of
      source-specific entry IDs is mapped.
    """
    if not path:
      return ()
    entry = self._entry_map[(source, path[0])]
    return (entry._canonical_id,) + entry.MapEntry(source, path[1:])

  def MergeEntries(self):
    """Recursively merge the entries in this map.

    Example: Suppose that the following entries were added to the map:

      map.AddEntry(source='trace_A.json', path=[10], name='Browser')
      map.AddEntry(source='trace_A.json', path=[20], name='Renderer')
      map.AddEntry(source='trace_B.json', path=[30], name='Browser')

    Before merging, |map._entry_map| will contain three separate items:

      ('trace_A.json', 10) -> IdMap(_items={id: {10}, name: {'Browser'}},
                                    _sources={'trace_A.json'})
      ('trace_A.json', 20) -> IdMap(_items={id: {20}, name: {'Renderer'}},
                                    _sources={'trace_A.json'})
      ('trace_B.json', 30) -> IdMap(_items={id: {30}, name: {'Browser'}},
                                    _sources={'trace_B.json'})

    Since the first two entries come from the same source, they cannot be
    merged. On the other hand, the third entry could be merged with either of
    the first two. Since it has a common name with the first entry, it will be
    merged with it in this method:

      ('trace_A.json', 10) -> IdMap(_items={id: {10, 30}, name: {'Browser'}},
                                    _sources={'trace_A.json', 'trace_B.json'})
      ('trace_A.json', 20) -> IdMap(_items={id: {20}, name: {Renderer}},
                                    _sources={'trace_A.json'})
      ('trace_B.json', 30) -> <same IdMap as ('trace_A.json', 10)>

    Pairs of entries will be merged in a descending order of sizes of
    pair-wise intersections of |entry._items| until there are no two entries
    such that (1) they have at least one value in |entry._items| in common and
    (2) they are mergeable (i.e. have no common source). Afterwards, unique IDs
    are assigned to the resulting "canonical" entries and their sub-entries are
    merged recursively.
    """
    # pylint: disable=unsubscriptable-object
    if self._depth == len(self.LEVELS):
      return

    logging.debug('Merging %r entries in %s...', self.LEVELS[self._depth].name,
                  self)

    canonical_entries = self._CanonicalizeEntries()
    self._AssignIdsToCanonicalEntries(canonical_entries)

    for entry in canonical_entries:
      entry.MergeEntries()

  def _GetSubMapEntry(self, source, entry_id):
    full_id = (source, entry_id)
    entry = self._entry_map.get(full_id)
    if entry is None:
      entry = type(self)(self._depth + 1)
      entry._sources.add(source)
      entry._items['id'].add(entry_id)
      self._entry_map[full_id] = entry
    return entry

  def _CalculateUnmergeableMapFromEntrySources(self):
    entry_ids_by_source = collections.defaultdict(set)
    for entry_id, entry in self._entry_map.items():
      for source in entry._sources:
        entry_ids_by_source[source].add(entry_id)

    unmergeable_map = collections.defaultdict(set)
    for unmergeable_set in entry_ids_by_source.values():
      for entry_id in unmergeable_set:
        unmergeable_map[entry_id].update(unmergeable_set - {entry_id})

    return unmergeable_map

  def _IsMergeableWith(self, other):
    return self._sources.isdisjoint(other._sources)

  def _GetMatch(self, other):
    # pylint: disable=unsubscriptable-object
    match_cls = self.LEVELS[self._depth - 1].match
    return match_cls(*(self._items[f] & other._items[f]
                       for f in match_cls._fields))

  def _MergeFrom(self, source):
    if self._depth > 0:
      # This is NOT a ROOT node, so we need to merge fields and sources from
      # the source node.
      for key, values in source._items.items():
        self._items[key].update(values)
      self._sources.update(source._sources)

    if self._depth < len(self.LEVELS):
      # This is NOT a LEAF node, so we need to copy over entries from the
      # source node's entry map.
      assert not set(self._entry_map) & set(source._entry_map)
      self._entry_map.update(source._entry_map)

  def _CanonicalizeEntries(self):
    canonical_entries = self._entry_map.copy()

    # {ID1, ID2} -> Match between the two entries.
    matches = {frozenset([full_id1, full_id2]): entry1._GetMatch(entry2)
               for full_id1, entry1 in canonical_entries.items()
               for full_id2, entry2 in canonical_entries.items()
               if entry1._IsMergeableWith(entry2)}

    while matches:
      # Pop the maximum match from the dictionary.
      max_match_set, max_match = max(
          list(matches.items()), key=lambda pair: [len(v) for v in pair[1]])
      del matches[max_match_set]
      canonical_full_id, merged_full_id = max_match_set

      # Skip pairs of entries that have nothing in common.
      if not any(max_match):
        continue

      # Merge the entries and update the map to reflect this.
      canonical_entry = canonical_entries[canonical_full_id]
      merged_entry = canonical_entries.pop(merged_full_id)
      logging.debug('Merging %s into %s [match=%s]...', merged_entry,
                    canonical_entry, max_match)
      canonical_entry._MergeFrom(merged_entry)
      del merged_entry
      self._entry_map[merged_full_id] = canonical_entry

      for match_set in matches.keys():
        if merged_full_id in match_set:
          # Remove other matches with the merged entry.
          del matches[match_set]
        elif canonical_full_id in match_set:
          [other_full_id] = match_set - {canonical_full_id}
          other_entry = canonical_entries[other_full_id]
          if canonical_entry._IsMergeableWith(other_entry):
            # Update other matches with the canonical entry which are still
            # mergeable.
            matches[match_set] = canonical_entry._GetMatch(other_entry)
          else:
            # Remove other matches with the canonical entry which have become
            # unmergeable.
            del matches[match_set]

    return list(canonical_entries.values())

  def _AssignIdsToCanonicalEntries(self, canonical_entries):
    assigned_ids = set()
    canonical_entries_without_assigned_ids = set()

    # Try to assign each canonical entry to one of the IDs from which it was
    # merged.
    for canonical_entry in canonical_entries:
      candidate_ids = canonical_entry._items['id']
      try:
        assigned_id = next(candidate_id for candidate_id in candidate_ids
                           if candidate_id not in assigned_ids)
      except StopIteration:
        canonical_entries_without_assigned_ids.add(canonical_entry)
        continue
      assigned_ids.add(assigned_id)
      canonical_entry._canonical_id = assigned_id

    # For canonical entries where this cannot be done (highly unlikely), scan
    # from the minimal merged ID upwards for the first unassigned ID.
    for canonical_entry in canonical_entries_without_assigned_ids:
      assigned_id = next(candidate_id for candidate_id in
                         itertools.count(min(canonical_entry._items['id']))
                         if candidate_id not in assigned_ids)
      assigned_ids.add(assigned_id)
      canonical_entry._canonical_id = assigned_id

  def __repr__(self):
    # pylint: disable=unsubscriptable-object
    cls_name = type(self).__name__
    if self._depth == 0:
      return '%s root' % cls_name
    return '%s %s entry(%s)' % (cls_name, self.LEVELS[self._depth - 1].name,
                                self._items)


class ProcessIdMap(IdMap):
  """Class for merging and mapping PIDs and TIDs from multiple sources."""

  LEVELS = (
      IdMapLevel(name='process',
                 match=collections.namedtuple('ProcessMatch',
                                              ['name', 'id', 'label'])),
      IdMapLevel(name='thread',
                 match=collections.namedtuple('ThreadMatch', ['name', 'id']))
  )


def LoadTrace(filename):
  """Load a trace from a (possibly gzipped) file and return its parsed JSON."""
  logging.info('Loading trace %r...', filename)
  if filename.endswith(HTML_FILENAME_SUFFIX):
    return LoadHTMLTrace(filename)
  if filename.endswith(GZIP_FILENAME_SUFFIX):
    with gzip.open(filename, 'rb') as f:
      return json.load(f)
  with open(filename, 'r') as f:
    return json.load(f)


def LoadHTMLTrace(filename):
  """Load a trace from a vulcanized HTML trace file."""
  trace_components = collections.defaultdict(list)

  with io.open(filename, 'r', encoding='utf-8') as file_handle:
    for sub_trace in html2trace.ReadTracesFromHTMLFile(file_handle):
      for name, component in TraceAsDict(sub_trace).items():
        trace_components[name].append(component)

  trace = {}
  for name, components in trace_components.items():
    if len(components) == 1:
      trace[name] = components[0]
    elif all(isinstance(component, list) for component in components):
      trace[name] = [e for component in components for e in component]
    else:
      trace[name] = components[0]
      logging.warning(
          'Values of repeated trace component %r in HTML trace %r are not '
          'lists. The first defined value of the component will be used.',
          filename, name)

  return trace


def SaveTrace(trace, filename):
  """Save a JSON trace to a (possibly gzipped) file."""
  if filename is None:
    logging.info('Dumping trace to standard output...')
    print(json.dumps(trace))
  else:
    logging.info('Saving trace %r...', filename)
    if filename.endswith(HTML_FILENAME_SUFFIX):
      with codecs.open(filename, mode='w', encoding='utf-8') as f:
        trace2html.WriteHTMLForTraceDataToFile([trace], 'Merged trace', f)
    elif filename.endswith(GZIP_FILENAME_SUFFIX):
      with gzip.open(filename, 'wb') as f:
        json.dump(trace, f)
    else:
      with open(filename, 'w') as f:
        json.dump(trace, f)


def TraceAsDict(trace):
  """Ensure that a trace is a dictionary."""
  if isinstance(trace, list):
    return {'traceEvents': trace}
  return trace


def MergeTraceFiles(input_trace_filenames, output_trace_filename):
  """Merge a collection of input trace files into an output trace file."""
  logging.info('Loading %d input traces...', len(input_trace_filenames))
  input_traces = collections.OrderedDict()
  for input_trace_filename in input_trace_filenames:
    input_traces[input_trace_filename] = LoadTrace(input_trace_filename)

  logging.info('Merging traces...')
  output_trace = MergeTraces(input_traces)

  logging.info('Saving output trace...')
  SaveTrace(output_trace, output_trace_filename)

  logging.info('Finished.')


def MergeTraces(traces):
  """Merge a collection of JSON traces into a single JSON trace."""
  trace_components = collections.defaultdict(collections.OrderedDict)

  for filename, trace in traces.items():
    for name, component in TraceAsDict(trace).items():
      trace_components[name][filename] = component

  merged_trace = {}
  for component_name, components_by_filename in trace_components.items():
    logging.info('Merging %d %r components...', len(components_by_filename),
                 component_name)
    merged_trace[component_name] = MergeComponents(component_name,
                                                   components_by_filename)

  return merged_trace


def MergeComponents(component_name, components_by_filename):
  """Merge a component of multiple JSON traces into a single component."""
  if component_name == 'traceEvents':
    return MergeTraceEvents(components_by_filename)
  return MergeGenericTraceComponents(component_name, components_by_filename)


def MergeTraceEvents(events_by_filename):
  """Merge trace events from multiple traces into a single list of events."""
  # Remove strings from the list of trace events
  # (https://github.com/catapult-project/catapult/issues/2497).
  events_by_filename = collections.OrderedDict(
      (filename, [e for e in events if not isinstance(e, StringTypes)])
      for filename, events in events_by_filename.items())

  timestamp_range_by_filename = _AdjustTimestampRanges(events_by_filename)
  process_map = _CreateProcessMapFromTraceEvents(events_by_filename)
  merged_events = _CombineTraceEvents(events_by_filename, process_map)
  _RemoveSurplusClockSyncEvents(merged_events)
  merged_events.extend(
      _BuildInjectedTraceMarkerEvents(timestamp_range_by_filename, process_map))
  return merged_events


def _RemoveSurplusClockSyncEvents(events):
  """Remove all clock sync events except for the first one."""
  # TODO(petrcermak): Figure out how to handle merging multiple clock sync
  # events.
  clock_sync_event_indices = [i for i, e in enumerate(events)
                              if e['ph'] == CLOCK_SYNC_EVENT_PHASE]
  # The indices need to be traversed from largest to smallest (hence the -1).
  for i in clock_sync_event_indices[:0:-1]:
    del events[i]


def _AdjustTimestampRanges(events_by_filename):
  logging.info('Adjusting timestamp ranges of traces...')

  previous_trace_max_timestamp = 0
  timestamp_range_by_filename = collections.OrderedDict()

  for index, (filename,
              events) in enumerate(list(events_by_filename.items()), 1):
    # Skip metadata events, the timestamps of which are always zero.
    non_metadata_events = [e for e in events if e['ph'] != METADATA_PHASE]
    if not non_metadata_events:
      logging.warning('Trace %r (%d/%d) only contains metadata events.',
                      filename, index, len(events_by_filename))
      timestamp_range_by_filename[filename] = None
      continue

    min_timestamp = min(e['ts'] for e in non_metadata_events)
    max_timestamp = max(e['ts'] for e in non_metadata_events)

    # Determine by how much the timestamps should be shifted.
    injected_timestamp_shift = max(
        previous_trace_max_timestamp + MIN_TRACE_GAP_IN_US - min_timestamp, 0)
    logging.info('Injected timestamp shift in trace %r (%d/%d): %d ms '
                 '[min=%d, max=%d, duration=%d].', filename, index,
                 len(events_by_filename), injected_timestamp_shift,
                 min_timestamp, max_timestamp, max_timestamp - min_timestamp)

    if injected_timestamp_shift > 0:
      # Shift the timestamps.
      for event in non_metadata_events:
        event['ts'] += injected_timestamp_shift

      # Adjust the range.
      min_timestamp += injected_timestamp_shift
      max_timestamp += injected_timestamp_shift

    previous_trace_max_timestamp = max_timestamp

    timestamp_range_by_filename[filename] = min_timestamp, max_timestamp

  return timestamp_range_by_filename


def _CreateProcessMapFromTraceEvents(events_by_filename):
  logging.info('Creating process map from trace events...')

  process_map = ProcessIdMap()
  for filename, events in events_by_filename.items():
    for event in events:
      pid, tid = event['pid'], event['tid']
      process_map.AddEntry(source=filename, path=(pid, tid))
      if event['ph'] == METADATA_PHASE:
        if event['name'] == 'process_name':
          process_map.AddEntry(source=filename, path=(pid,),
                               name=event['args']['name'])
        elif event['name'] == 'process_labels':
          process_map.AddEntry(source=filename, path=(pid,),
                               label=event['args']['labels'].split(','))
        elif event['name'] == 'thread_name':
          process_map.AddEntry(source=filename, path=(pid, tid),
                               name=event['args']['name'])

  process_map.MergeEntries()
  return process_map


def _CombineTraceEvents(events_by_filename, process_map):
  logging.info('Combining trace events from all traces...')

  type_name_event_by_pid = {}
  combined_events = []

  for index, (filename,
              events) in enumerate(list(events_by_filename.items()), 1):
    for event in events:
      if _UpdateTraceEventForMerge(event, process_map, filename, index,
                                   type_name_event_by_pid):
        combined_events.append(event)

  return combined_events


def _UpdateTraceEventForMerge(event, process_map, filename, index,
                              type_name_event_by_pid):
  pid, tid = process_map.MapEntry(source=filename,
                                  path=(event['pid'], event['tid']))
  event['pid'], event['tid'] = pid, tid

  if event['ph'] == METADATA_PHASE:
    # Update IDs in 'stackFrames' and 'typeNames' metadata events.
    if event['name'] == 'stackFrames':
      _UpdateDictIds(index, event['args'], 'stackFrames')
      for frame in event['args']['stackFrames'].values():
        _UpdateFieldId(index, frame, 'parent')
    elif event['name'] == 'typeNames':
      _UpdateDictIds(index, event['args'], 'typeNames')
      existing_type_name_event = type_name_event_by_pid.get(pid)
      if existing_type_name_event is None:
        type_name_event_by_pid[pid] = event
      else:
        existing_type_name_event['args']['typeNames'].update(
            event['args']['typeNames'])
        # Don't add the event to the merged trace because it has been merged
        # into an existing 'typeNames' metadata event for the given process.
        return False

  elif event['ph'] == MEMORY_DUMP_PHASE:
    # Update stack frame and type name IDs in heap dump entries in process
    # memory dumps.
    for heap_dump in event['args']['dumps'].get('heaps', {}).values():
      for heap_entry in heap_dump['entries']:
        _UpdateFieldId(index, heap_entry, 'bt', ignored_values=[''])
        _UpdateFieldId(index, heap_entry, 'type')

  return True  # Events should be added to the merged trace by default.


def _ConvertId(index, original_id):
  return '%d#%s' % (index, original_id)


def _UpdateDictIds(index, parent_dict, key):
  parent_dict[key] = {
      _ConvertId(index, original_id): value
      for original_id, value in parent_dict[key].items()}


def _UpdateFieldId(index, parent_dict, key, ignored_values=()):
  original_value = parent_dict.get(key)
  if original_value is not None and original_value not in ignored_values:
    parent_dict[key] = _ConvertId(index, original_value)


def _BuildInjectedTraceMarkerEvents(timestamp_range_by_filename, process_map):
  logging.info('Building injected trace marker events...')

  injected_pid = process_map.max_mapped_id + 1

  # Inject a mock process with a thread.
  injected_events = [
      {
          'pid': injected_pid,
          'tid': 0,
          'ph': METADATA_PHASE,
          'ts': 0,
          'name': 'process_sort_index',
          'args': {'sort_index': -1000}  # Show the process at the top.
      },
      {
          'pid': injected_pid,
          'tid': 0,
          'ph': METADATA_PHASE,
          'ts': 0,
          'name': 'process_name',
          'args': {'name': 'Merged traces'}
      },
      {
          'pid': injected_pid,
          'tid': 0,
          'ph': METADATA_PHASE,
          'ts': 0,
          'name': 'thread_name',
          'args': {'name': 'Trace'}
      }
  ]

  # Inject slices for each sub-trace denoting its beginning and end.
  for index, (filename, timestamp_range) in enumerate(
      list(timestamp_range_by_filename.items()), 1):
    if timestamp_range is None:
      continue
    min_timestamp, max_timestamp = timestamp_range
    name = 'Trace %r (%d/%d)' % (filename, index,
                                 len(timestamp_range_by_filename))
    slice_id = 'INJECTED_TRACE_MARKER_%d' % index
    injected_events.extend([
        {
            'pid': injected_pid,
            'tid': 0,
            'ph': BEGIN_PHASE,
            'cat': 'injected',
            'name': name,
            'id': slice_id,
            'ts': min_timestamp
        },
        {
            'pid': injected_pid,
            'tid': 0,
            'ph': END_PHASE,
            'cat': 'injected',
            'name': name,
            'id': slice_id,
            'ts': max_timestamp
        }
    ])

  return injected_events


def MergeGenericTraceComponents(component_name, components_by_filename):
  """Merge a generic component of multiple JSON traces into a single component.

  This function is only used for components that don't have a component-specific
  merging function (see MergeTraceEvents). It just returns the component's first
  provided value (in some trace).
  """
  components = list(components_by_filename.values())
  first_component = next(iter(components))
  if not all(c == first_component for c in components):
    logging.warning(
        'Values of trace component %r differ across the provided traces. '
        'The first defined value of the component will be used.',
        component_name)
  return first_component


def Main(argv):
  parser = argparse.ArgumentParser(description='Merge multiple traces.',
                                   add_help=False)
  parser.add_argument('input_traces', metavar='INPUT_TRACE', nargs='+',
                      help='Input trace filename.')
  parser.add_argument('-h', '--help', action='help',
                      help='Show this help message and exit.')
  parser.add_argument('-o', '--output_trace', help='Output trace filename. If '
                      'not provided, the merged trace will be written to '
                      'the standard output.')
  parser.add_argument('-v', '--verbose', action='count', dest='verbosity',
                      help='Increase verbosity level.')
  args = parser.parse_args(argv[1:])

  # Set verbosity level.
  if args.verbosity >= 2:
    logging_level = logging.DEBUG
  elif args.verbosity == 1:
    logging_level = logging.INFO
  else:
    logging_level = logging.WARNING
  logging.getLogger().setLevel(logging_level)

  try:
    MergeTraceFiles(args.input_traces, args.output_trace)
    return 0
  except Exception:  # pylint: disable=broad-except
    logging.exception('Something went wrong:')
    return 1
  finally:
    logging.warning('This is an EXPERIMENTAL TOOL! If you encounter any '
                    'issues, please file a Catapult bug '
                    '(https://github.com/catapult-project/catapult/issues/new) '
                    'with your current Catapult commit hash, a description of '
                    'the problem and any error messages, attach the input '
                    'traces and notify petrcermak@chromium.org. Thank you!')
