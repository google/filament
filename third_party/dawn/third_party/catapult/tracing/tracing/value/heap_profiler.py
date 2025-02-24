# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import codecs
import collections
import sys
import time

import six
from tracing.value import histogram
from tracing.value import histogram_set
from tracing.value.diagnostics import breakdown
from tracing.value.diagnostics import date_range
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import related_name_map
from tracing.value.diagnostics import reserved_infos
from tracing_build import render_histograms_viewer


def _IsUserDefinedInstance(obj):
  return type(obj).__module__ != six.moves.builtins.__name__


class _HeapProfiler(object):
  __slots__ = '_diagnostics_callback', '_histograms', '_seen'

  def __init__(self, diagnostics_callback=None):
    self._diagnostics_callback = diagnostics_callback
    self._histograms = None
    self._seen = set()

  def Profile(self, root):
    self._histograms = histogram_set.HistogramSet()
    total_hist = self._GetOrCreateHistogram('heap')
    total_hist.diagnostics['types'] = related_name_map.RelatedNameMap()
    total_breakdown = breakdown.Breakdown()
    total_size = self._Recurse(
        root, total_hist.diagnostics['types'], total_breakdown)
    builtins_size = total_size - sum(subsize for _, subsize in total_breakdown)

    if builtins_size:
      total_breakdown.Set('(builtin types)', builtins_size)
    total_hist.AddSample(total_size, dict(types=total_breakdown))

    self._histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.TRACE_START.name,
        date_range.DateRange(time.time() * 1000))

    return self._histograms

  def _GetOrCreateHistogram(self, name):
    hs = self._histograms.GetHistogramsNamed(name)
    if len(hs) > 1:
      raise Exception('Too many Histograms named %s' % name)

    if len(hs) == 1:
      return hs[0]

    hist = histogram.Histogram(name, 'sizeInBytes_smallerIsBetter')
    hist.CustomizeSummaryOptions(dict(std=False, min=False, max=False))
    self._histograms.AddHistogram(hist)
    return hist

  def _Recurse(self, obj, parent_related_names, parent_breakdown):
    if id(obj) in self._seen:
      return 0
    self._seen.add(id(obj))

    size = sys.getsizeof(obj)

    related_names = parent_related_names
    types_breakdown = parent_breakdown

    hist = None
    if _IsUserDefinedInstance(obj):
      type_name = type(obj).__name__
      hist = self._GetOrCreateHistogram('heap:' + type_name)

      related_names = hist.diagnostics.get('types')
      if related_names is None:
        related_names = related_name_map.RelatedNameMap()
      types_breakdown = breakdown.Breakdown()

    if isinstance(obj, dict):
      for objkey, objvalue in six.iteritems(obj):
        size += self._Recurse(objkey, related_names, types_breakdown)
        size += self._Recurse(objvalue, related_names, types_breakdown)
    elif isinstance(obj, (tuple, list, set, frozenset, collections.deque)):
      # Can't use collections.Iterable because strings are iterable, but
      # sys.getsizeof() already handles strings, we don't need to iterate over
      # them.
      for elem in obj:
        size += self._Recurse(elem, related_names, types_breakdown)

    # It is possible to subclass builtin types like dict and add properties to
    # them, so handle __dict__ and __slots__ even if obj is a dict/list/etc.

    properties_breakdown = breakdown.Breakdown()
    if hasattr(obj, '__dict__'):
      size += sys.getsizeof(obj.__dict__)
      for dkey, dvalue in six.iteritems(obj.__dict__):
        size += self._Recurse(dkey, related_names, types_breakdown)
        dsize = self._Recurse(dvalue, related_names, types_breakdown)
        properties_breakdown.Set(dkey, dsize)
        size += dsize
      size += self._Recurse(obj.__dict__, related_names, types_breakdown)

    # It is possible for a class to use both __slots__ and __dict__ by listing
    # __dict__ as a slot.

    if hasattr(obj.__class__, '__slots__'):
      for slot in obj.__class__.__slots__:
        if slot == '__dict__':
          # obj.__dict__ was already handled
          continue
        if not hasattr(obj, slot):
          continue
        slot_size = self._Recurse(
            getattr(obj, slot), related_names, types_breakdown)
        properties_breakdown.Set(slot, slot_size)
        size += slot_size

    if hist:
      if len(related_names):
        hist.diagnostics['types'] = related_names

      parent_related_names.Set(type_name, hist.name)
      parent_breakdown.Set(type_name, parent_breakdown.Get(type_name) + size)

      builtins_size = size - sum(subsize for _, subsize in types_breakdown)
      if builtins_size:
        types_breakdown.Set('(builtin types)', builtins_size)

      sample_diagnostics = {'types': types_breakdown}
      if len(properties_breakdown):
        sample_diagnostics['properties'] = properties_breakdown
      if self._diagnostics_callback:
        sample_diagnostics.update(self._diagnostics_callback(obj))

      hist.AddSample(size, sample_diagnostics)

    return size


def Profile(root, label=None, html_filename=None, html_stream=None,
            vulcanized_viewer=None, reset_results=False,
            diagnostics_callback=None):
  """Profiles memory consumed by the root object.

  Produces a HistogramSet containing 1 Histogram for each user-defined class
  encountered when recursing through the root object's properties.
  Each Histogram contains 1 sample for each instance of the class.
  Each sample contains 2 Breakdowns:
  - 'types' allows drilling down into memory profiles for other classes, and
  - 'properties' breaks down the size of an instance by its properties.

  Args:
      label: string label to distinguish these results from those produced by
          other Profile() calls.
      html_filename: string filename to write HTML results.
      html_stream: file-like string to write HTML results.
      vulcanized_viewer: HTML string
      reset_results: whether to delete pre-existing results in
          html_filename/html_stream
      diagnostics_callback: function that takes an instance of a class, and
          returns a dictionary from strings to Diagnostic objects.

  Returns:
      HistogramSet
  """
  # TODO(4068): Package this and its dependencies and a vulcanized viewer in
  # order to remove the vulcanized_viewer parameter and simplify rendering the
  # viewer.

  profiler = _HeapProfiler(diagnostics_callback)
  histograms = profiler.Profile(root)

  if label:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.LABELS.name, generic_set.GenericSet([label]))

  if html_filename and not html_stream:
    open(html_filename, 'a').close()  # Create file if it doesn't exist.
    html_stream = codecs.open(html_filename, mode='r+', encoding='utf-8')

  if html_stream:
    # Vulcanizing the viewer requires a full catapult checkout, which is not
    # available in some contexts such as appengine.
    # Merely rendering the viewer requires a pre-vulcanized viewer HTML string.
    # render_histograms_viewer does not require a full checkout, so it can run
    # in restricted contexts such as appengine as long as a pre-vulcanized
    # viewer is provided.
    if vulcanized_viewer:
      render_histograms_viewer.RenderHistogramsViewer(
          histograms.AsDicts(), html_stream, reset_results, vulcanized_viewer)
    else:
      from tracing_build import vulcanize_histograms_viewer # pylint: disable=import-outside-toplevel
      vulcanize_histograms_viewer.VulcanizeAndRenderHistogramsViewer(
          histograms.AsDicts(), html_stream)

  return histograms
