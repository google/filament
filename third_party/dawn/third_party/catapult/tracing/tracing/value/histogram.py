# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import json
import math
import numbers
import pprint
import random

import six
from six.moves import range  # pylint: disable=redefined-builtin
from tracing.proto import histogram_proto
from tracing.value.diagnostics import diagnostic
from tracing.value.diagnostics import diagnostic_ref
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos
from tracing.value.diagnostics import unmergeable_diagnostic_set


try:
  StringTypes = six.string_types # pylint: disable=invalid-name
except NameError:
  StringTypes = str


# pylint: disable=too-many-lines
# TODO(#3613) Split this file.


# This should be equal to sys.float_info.max, but that value might differ
# between platforms, whereas ECMA Script specifies this value for all platforms.
# The specific value should not matter in normal practice.
JS_MAX_VALUE = 1.7976931348623157e+308
DEFAULT_ITERATION_FOR_BOOTSTRAP_RESAMPLING = 500


# Converts the given percent to a string in the following format:
# 0.x produces '0x0',
# 0.xx produces '0xx',
# 0.xxy produces '0xx_y',
# 1.0 produces '100'.
def PercentToString(percent, force3=False):
  if percent < 0 or percent > 1:
    raise ValueError('percent must be in [0,1]')
  if percent == 0:
    return '000'
  if percent == 1:
    return '100'
  s = str(percent)
  if s[1] != '.':
    raise ValueError('Unexpected percent')
  s += '0' * max(4 - len(s), 0)
  if len(s) > 4:
    if force3:
      s = s[:4]
    else:
      s = s[:4] + '_' + s[4:]
  return '0' + s[2:]


def PercentFromString(s):
  return float(s[0] + '.' + s[1:].replace('_', ''))


# This variation of binary search returns the index |hi| into |ary| for which
# callback(ary[hi]) < 0 and callback(ary[hi-1]) >= 0
# This function assumes that map(callback, ary) is sorted descending.
def FindHighIndexInSortedArray(ary, callback):
  lo = 0
  hi = len(ary)
  while lo < hi:
    mid = (lo + hi) >> 1
    if callback(ary[mid]) >= 0:
      lo = mid + 1
    else:
      hi = mid
  return hi


# Modifies |samples| in-place to reduce its length to |count|, discarding random
# elements.
def UniformlySampleArray(samples, count):
  while len(samples) > count:
    samples.pop(int(random.uniform(0, len(samples))))
  return samples


# When processing a stream of samples, call this method for each new sample in
# order to decide whether to keep it in |samples|.
# Modifies |samples| in-place such that its length never exceeds |num_samples|.
# After |stream_length| samples have been processed, each sample has equal
# probability of being retained in |samples|.
# The order of samples is not preserved after |stream_length| exceeds
# |num_samples|.
def UniformlySampleStream(samples, stream_length, new_element, num_samples):
  if stream_length <= num_samples:
    if len(samples) >= stream_length:
      samples[stream_length - 1] = new_element
    else:
      samples.append(new_element)
    return

  prob_keep = num_samples / stream_length
  if random.random() > prob_keep:
    # reject new_sample
    return

  # replace a random element
  samples[int(math.floor(random.random() * num_samples))] = new_element


# Merge two sets of samples that were assembled using UniformlySampleStream().
# Modify |a_samples| in-place such that all of the samples in |a_samples| and
# |b_samples| have equal probability of being retained in |a_samples|.
def MergeSampledStreams(a_samples, a_stream_length,
                        b_samples, b_stream_length, num_samples):
  if b_stream_length < num_samples:
    for i in range(min(b_stream_length, len(b_samples))):
      UniformlySampleStream(
          a_samples, a_stream_length + i + 1, b_samples[i], num_samples)
    return

  if a_stream_length < num_samples:
    temp_samples = list(b_samples)
    for i in range(min(a_stream_length, len(a_samples))):
      UniformlySampleStream(
          temp_samples, b_stream_length + i + 1, a_samples[i], num_samples)
    for i, temp_sample in enumerate(temp_samples):
      a_samples[i] = temp_sample
    return

  prob_swap = b_stream_length / (a_stream_length + b_stream_length)
  for i in range(min(num_samples, len(b_samples))):
    if random.random() < prob_swap:
      a_samples[i] = b_samples[i]


def Percentile(ary, percent):
  if percent < 0 or percent > 1:
    raise ValueError('percent must be in [0,1]')
  ary = list(ary)
  ary.sort()
  return ary[int((len(ary) - 1) * percent)]


class HistogramError(ValueError):
  """Base execption type for Histogram related exceptions."""


class InvalidBucketError(HistogramError):
  """Context-carrying exception type for invalid bucket values."""

  def __init__(self, bucket_index, value, buckets):
    """Raised when a provided index to a bucket is not valid.

    Arguments:
      - bucket_index: a numeric index.
      - value: a dict associated with the provided bucket index.
      - buckets: a list of valid bucket definitions.
    """
    super(InvalidBucketError, self).__init__()
    self.bucket_index = bucket_index
    self.value = value
    self.buckets = buckets

  def __str__(self):
    return 'Invalid Bucket: %s not a valid offset from [0..%s); value = %r' % (
        self.bucket_index, len(self.buckets), self.value)


class Range(object):
  __slots__ = '_empty', '_min', '_max'

  def __init__(self):
    self._empty = True
    self._min = None
    self._max = None

  def __eq__(self, other):
    if not isinstance(other, Range):
      return False
    if self.empty and other.empty:
      return True
    if self.empty != other.empty:
      return False
    return  (self.min == other.min) and (self.max == other.max)

  def __hash__(self):
    return id(self)

  @staticmethod
  def FromExplicitRange(lower, upper):
    r = Range()
    r._min = lower
    r._max = upper
    r._empty = False
    return r

  @property
  def empty(self):
    return self._empty

  @property
  def min(self):
    return self._min

  @property
  def max(self):
    return self._max

  @property
  def center(self):
    return (self._min + self._max) * 0.5

  @property
  def duration(self):
    if self.empty:
      return 0
    return self._max - self._min

  def AddValue(self, x):
    if self._empty:
      self._empty = False
      self._min = x
      self._max = x
      return

    self._max = max(x, self._max)
    self._min = min(x, self._min)

  def AddRange(self, other):
    if other.empty:
      return
    self.AddValue(other.min)
    self.AddValue(other.max)


# This class computes statistics online in O(1).
class RunningStatistics(object):
  __slots__ = (
      '_count', '_mean', '_max', '_min', '_sum', '_variance', '_meanlogs')

  def __init__(self):
    self._count = 0
    self._mean = 0.0
    self._max = -JS_MAX_VALUE
    self._min = JS_MAX_VALUE
    self._sum = 0.0
    self._variance = 0.0
    # Mean of logarithms of samples, or None if any samples were <= 0.
    self._meanlogs = 0.0

  @property
  def count(self):
    return self._count

  @property
  def geometric_mean(self):
    if self._meanlogs is None:
      return None
    return math.exp(self._meanlogs)

  @property
  def mean(self):
    if self._count == 0:
      return None
    return self._mean

  @property
  def max(self):
    return self._max

  @property
  def min(self):
    return self._min

  @property
  def sum(self):
    return self._sum

  # This returns the variance of the samples after Bessel's correction has
  # been applied.
  @property
  def variance(self):
    if self.count == 0 or self._variance is None:
      return None
    if self.count == 1:
      return 0
    return self._variance / (self.count - 1)

  # This returns the standard deviation of the samples after Bessel's
  # correction has been applied.
  @property
  def stddev(self):
    if self.count == 0 or self._variance is None:
      return None
    return math.sqrt(self.variance)

  def Add(self, x):
    self._count += 1
    x = float(x)
    self._max = max(self._max, x)
    self._min = min(self._min, x)
    self._sum += x

    if x <= 0.0:
      self._meanlogs = None
    elif self._meanlogs is not None:
      self._meanlogs += (math.log(abs(x)) - self._meanlogs) / self.count

    # The following uses Welford's algorithm for computing running mean and
    # variance. See http://www.johndcook.com/blog/standard_deviation.
    if self.count == 1:
      self._mean = x
      self._variance = 0.0
    else:
      old_mean = self._mean
      old_variance = self._variance

      # Using the 2nd formula for updating the mean yields better precision but
      # it doesn't work for the case oldMean is Infinity. Hence we handle that
      # case separately.
      if abs(old_mean) == float('inf'):
        self._mean = self._sum / self.count
      else:
        self._mean = old_mean + float(x - old_mean) / self.count
      self._variance = old_variance + (x - old_mean) * (x - self._mean)

  def Merge(self, other):
    result = RunningStatistics()
    result._count = self._count + other._count
    result._sum = self._sum + other._sum
    result._min = min(self._min, other._min)
    result._max = max(self._max, other._max)
    if result._count == 0:
      result._mean = 0.0
      result._variance = 0.0
      result._meanlogs = 0.0
    else:
      # Combine the mean and the variance using the formulas from
      # https://goo.gl/ddcAep.
      result._mean = float(result._sum) / result._count
      delta_mean = (self._mean or 0.0) - (other._mean or 0.0)
      result._variance = self._variance + other._variance + (
          self._count * other._count * delta_mean * delta_mean / result._count)

      # Merge the arithmetic means of logarithms of absolute values of samples,
      # weighted by counts.
      if self._meanlogs is None or other._meanlogs is None:
        result._meanlogs = None
      else:
        result._meanlogs = (self._count * self._meanlogs +
                            other._count * other._meanlogs) / result._count
    return result

  def AsDict(self):
    if self._count == 0:
      return []

    # Javascript automatically converts between ints and floats.
    # It's more efficient to serialize integers as ints than floats.
    def FloatAsFloatOrInt(x):
      if x is not None and x.is_integer():
        return int(x)
      return x

    # It's more efficient to serialize these fields in an array. If you add any
    # other fields, you should re-evaluate whether it would be more efficient to
    # serialize as a dict.
    return [
        self._count,
        FloatAsFloatOrInt(self._max),
        FloatAsFloatOrInt(self._meanlogs),
        FloatAsFloatOrInt(self._mean),
        FloatAsFloatOrInt(self._min),
        FloatAsFloatOrInt(self._sum),
        FloatAsFloatOrInt(self._variance),
    ]

  def AsProto(self):
    proto = histogram_proto.Pb2().RunningStatistics()

    if self._count == 0:
      return proto

    # We can't currently set None; setting 0 at least means nothing is sent
    # on the wire.
    proto.count = self._count or 0
    proto.max = self._max or 0
    proto.meanlogs = self._meanlogs or 0
    proto.mean = self._mean or 0
    proto.min = self._min or 0
    proto.sum = self._sum or 0
    proto.variance = self._variance or 0

    return proto

  @staticmethod
  def FromDict(dct):
    result = RunningStatistics()
    if len(dct) != 7:
      return result

    def AsFloatOrNone(x):
      if x is None:
        return x
      return float(x)
    [result._count, result._max, result._meanlogs, result._mean, result._min,
     result._sum, result._variance] = [int(dct[0])] + [
         AsFloatOrNone(x) for x in dct[1:]]
    return result

  @staticmethod
  def FromProto(proto):
    result = RunningStatistics()

    # TODO(http://crbug.com/1029452): Make these IntValue in the proto, since
    # we apparently need to be able to tell None from 0.
    result._count = proto.count
    result._max = proto.max
    result._meanlogs = proto.meanlogs
    result._mean = proto.mean
    result._min = proto.min
    result._sum = proto.sum
    result._variance = proto.variance

    return result


class DiagnosticMap(dict):
  __slots__ = ('_allow_reserved_names',)

  def __init__(self, *args, **kwargs):
    self._allow_reserved_names = True
    dict.__init__(self, *args, **kwargs)

  def DisallowReservedNames(self):
    self._allow_reserved_names = False

  def __setitem__(self, name, diag):
    if not isinstance(name, StringTypes):
      raise TypeError('name must be string')
    if not isinstance(diag, (diagnostic.Diagnostic,
                             diagnostic_ref.DiagnosticRef)):
      raise TypeError('diag must be Diagnostic or DiagnosticRef')
    if (not self._allow_reserved_names and
        not isinstance(diag,
                       unmergeable_diagnostic_set.UnmergeableDiagnosticSet) and
        not isinstance(diag, diagnostic_ref.DiagnosticRef)):
      expected_type = reserved_infos.GetTypeForName(name)
      if expected_type and diag.__class__.__name__ != expected_type:
        raise TypeError('Diagnostics names "%s" must be %s, not %s' %
                        (name, expected_type, diag.__class__.__name__))
    dict.__setitem__(self, name, diag)

  @staticmethod
  def Deserialize(data, deserializer):
    dm = DiagnosticMap()
    dm.DeserializeAdd(data, deserializer)
    return dm

  def DeserializeAdd(self, data, deserializer):
    for i in data:
      self.update(deserializer.GetDiagnostic(i))

  @staticmethod
  def FromDict(dct):
    dm = DiagnosticMap()
    dm.AddDicts(dct)
    return dm

  @staticmethod
  def FromProto(proto):
    dm = DiagnosticMap()
    dm.AddProtos(proto.diagnostic_map)
    return dm

  def AddDicts(self, dct):
    for name, diagnostic_dict in dct.items():
      if name == 'tagmap':
        continue
      if isinstance(diagnostic_dict, StringTypes):
        self[name] = diagnostic_ref.DiagnosticRef(diagnostic_dict)
      elif diagnostic_dict['type'] not in [
          'RelatedHistogramMap', 'RelatedHistogramBreakdown', 'TagMap']:
        # Ignore RelatedHistograms and TagMaps.
        # TODO(benjhayden): Forget about them in 2019 Q2.
        self[name] = diagnostic.Diagnostic.FromDict(diagnostic_dict)

  def AddProtos(self, protos):
    for name, diagnostic_proto in protos.items():
      if diagnostic_proto.HasField('shared_diagnostic_guid'):
        self[name] = diagnostic_ref.DiagnosticRef(
            diagnostic_proto.shared_diagnostic_guid)
      else:
        self[name] = diagnostic.Diagnostic.FromProto(diagnostic_proto)

  def ResolveSharedDiagnostics(self, histograms, required=False):
    for name, diag in self.items():
      if not isinstance(diag, diagnostic_ref.DiagnosticRef):
        continue
      guid = diag.guid
      diag = histograms.LookupDiagnostic(guid)
      if isinstance(diag, diagnostic.Diagnostic):
        self[name] = diag
      elif required:
        raise ValueError('Unable to find shared Diagnostic ' + guid)

  def Serialize(self, serializer):
    return [serializer.GetOrAllocateDiagnosticId(name, diag)
            for name, diag in self.items()]

  def AsDict(self):
    dct = {}
    for name, diag in self.items():
      dct[name] = diag.AsDictOrReference()
    return dct

  def AsProto(self, proto):
    for name, diag in self.items():
      proto.diagnostic_map[name].CopyFrom(diag.AsProtoOrReference())
    return proto

  def Merge(self, other):
    for name, other_diagnostic in other.items():
      if name not in self:
        self[name] = other_diagnostic
        continue
      my_diagnostic = self[name]
      if my_diagnostic.CanAddDiagnostic(other_diagnostic):
        my_diagnostic.AddDiagnostic(other_diagnostic)
        continue
      self[name] = unmergeable_diagnostic_set.UnmergeableDiagnosticSet([
          my_diagnostic, other_diagnostic])


MAX_DIAGNOSTIC_MAPS = 16


class HistogramBin(object):
  __slots__ = '_range', '_count', '_diagnostic_maps'

  def __init__(self, rang):
    self._range = rang
    self._count = 0
    self._diagnostic_maps = []

  def AddSample(self, unused_x):
    self._count += 1

  @property
  def count(self):
    return self._count

  @property
  def range(self):
    return self._range

  def AddBin(self, other):
    self._count += other.count

  @property
  def diagnostic_maps(self):
    return self._diagnostic_maps

  def AddDiagnosticMap(self, diagnostics):
    UniformlySampleStream(
        self._diagnostic_maps, self.count, diagnostics, MAX_DIAGNOSTIC_MAPS)

  def FromDict(self, dct):
    self._count = dct[0]
    if len(dct) > 1:
      for diagnostic_map_dict in dct[1]:
        self._diagnostic_maps.append(DiagnosticMap.FromDict(
            diagnostic_map_dict))

  def FromProto(self, proto):
    self._count = proto.bin_count

    for diagnostic_map in proto.diagnostic_maps:
      self._diagnostic_maps.append(DiagnosticMap.FromProto(diagnostic_map))

  def AsDict(self):
    if len(self._diagnostic_maps) == 0:
      return [self.count]
    return [self.count, [d.AsDict() for d in self._diagnostic_maps]]

  def AsProto(self):
    proto = histogram_proto.Pb2().Bin()
    proto.bin_count = self._count
    if len(self._diagnostic_maps) == 0:
      return proto

    proto.diagnostic_maps.extend([d.AsProto() for d in self._diagnostic_maps])
    return proto

  def Serialize(self, serializer):
    if len(self._diagnostic_maps) == 0:
      return self.count
    return [self.count] + [
        [None] + d.Serialize(serializer) for d in self._diagnostic_maps]

  def Deserialize(self, data, deserializer):
    if not isinstance(data, list):
      self._count = data
      return
    self._count = data[0]
    for sample in data[1:]:
      # TODO(benjhayden): class Sample
      if not isinstance(sample, list):
        continue
      self._diagnostic_maps.append(DiagnosticMap.Deserialize(
          sample[1:], deserializer))


# This list should be kept in sync with tracing/tracing/base/unit.html
# and tracing/tracing/value/histogram.cc.
# TODO(#3814) Presubmit to compare with unit.html.
UNIT_NAMES = [
    'ms',
    'msBestFitFormat',
    'tsMs',
    'n%',
    'sizeInBytes',
    'bytesPerSecond',
    'J',  # Joule
    'W',  # Watt
    'A',  # Ampere
    'Ah',  # Ampere-hours
    'V',  # Volt
    'Hz',  # Hertz
    'unitless',
    'count',
    'sigma',
]

def ExtendUnitNames():
  # Use a function in order to avoid cluttering the global namespace with a loop
  # variable.
  for name in list(UNIT_NAMES):
    UNIT_NAMES.append(name + '_biggerIsBetter')
    UNIT_NAMES.append(name + '_smallerIsBetter')
    UNIT_NAMES.append(name + '-')
    UNIT_NAMES.append(name + '+')

ExtendUnitNames()

def UnrecognizedUnitMessage(unit):
  output = 'Unrecognized unit "%r". ' % unit
  output += "Use one of the following:\n"
  output += pprint.pformat(UNIT_NAMES)
  return output


class Scalar(object):
  __slots__ = '_unit', '_value'

  def __init__(self, unit, value):
    assert unit in UNIT_NAMES, UnrecognizedUnitMessage(unit)
    self._unit = unit
    self._value = value

  @property
  def unit(self):
    return self._unit

  @property
  def value(self):
    return self._value

  def AsDict(self):
    return {'type': 'scalar', 'unit': self.unit, 'value': self.value}

  @staticmethod
  def FromDict(dct):
    return Scalar(dct['unit'], dct['value'])


DEFAULT_SUMMARY_OPTIONS = {
    'avg': True,
    'geometricMean': False,
    'std': True,
    'count': True,
    'sum': True,
    'min': True,
    'max': True,
    'nans': False,
    # Don't include 'percentile' here. Its default value is [], which is
    # modifiable. Callers may push to it, so there must be a different Array
    # instance for each Histogram instance.
}


class Histogram(object):
  __slots__ = (
      '_bin_boundaries_dict',
      '_description',
      '_name',
      '_diagnostics',
      '_nan_diagnostic_maps',
      '_num_nans',
      '_running',
      '_sample_values',
      '_sample_means',
      '_summary_options',
      '_unit',
      '_bins',
      '_max_num_sample_values')

  def __init__(self, name, unit, bin_boundaries=None):
    assert unit in UNIT_NAMES, UnrecognizedUnitMessage(unit)

    if bin_boundaries is None:
      base_unit = unit.split('_')[0].strip('+-')
      bin_boundaries = DEFAULT_BOUNDARIES_FOR_UNIT[base_unit]

    # Serialize bin boundaries here instead of holding a reference to it in case
    # it is modified.
    self._bin_boundaries_dict = bin_boundaries.AsDict()

    # HistogramBinBoundaries creates empty HistogramBins. Save memory by sharing
    # those empty HistogramBin instances with other Histograms. Wait to copy
    # HistogramBins until we need to modify it (copy-on-write).
    self._bins = list(bin_boundaries.bins)
    self._description = ''
    self._name = name
    self._diagnostics = DiagnosticMap()
    self._diagnostics.DisallowReservedNames()
    self._nan_diagnostic_maps = []
    self._num_nans = 0
    self._running = None
    self._sample_values = []
    self._sample_means = []
    self._summary_options = dict(DEFAULT_SUMMARY_OPTIONS)
    self._summary_options['percentile'] = []
    self._summary_options['ci'] = []
    self._unit = unit

    self._max_num_sample_values = self._GetDefaultMaxNumSampleValues()

  @classmethod
  def Create(cls, name, unit, samples, bin_boundaries=None, description='',
             summary_options=None, diagnostics=None):
    hist = cls(name, unit, bin_boundaries)
    hist.description = description
    if summary_options:
      hist.CustomizeSummaryOptions(summary_options)
    if diagnostics:
      for diag_name, diag in diagnostics.items():
        hist.diagnostics[diag_name] = diag

    if not isinstance(samples, list):
      samples = [samples]
    for sample in samples:
      if isinstance(sample, tuple) and len(sample) == 2:
        hist.AddSample(sample[0], sample[1])
      else:
        hist.AddSample(sample)

    return hist

  @property
  def description(self):
    return self._description

  @description.setter
  def description(self, d):
    self._description = d

  @property
  def nan_diagnostic_maps(self):
    return self._nan_diagnostic_maps

  @property
  def unit(self):
    return self._unit

  @property
  def running(self):
    return self._running

  @property
  def max_num_sample_values(self):
    return self._max_num_sample_values

  @max_num_sample_values.setter
  def max_num_sample_values(self, n):
    self._max_num_sample_values = n
    UniformlySampleArray(self._sample_values, self._max_num_sample_values)

  @property
  def sample_values(self):
    return self._sample_values

  @property
  def name(self):
    return self._name

  @property
  def bins(self):
    return self._bins

  @property
  def diagnostics(self):
    return self._diagnostics

  def _DeserializeStatistics(self):
    statistics_names = self.diagnostics.get(
        reserved_infos.STATISTICS_NAMES.name)
    if not statistics_names:
      return
    for stat_name in statistics_names:
      if stat_name.startswith('pct_'):
        percent = PercentFromString(stat_name[4:])
        self._summary_options.get('percentile').append(percent)
      elif stat_name.startswith('ipr_'):
        lower = PercentFromString(stat_name[4:7])
        upper = PercentFromString(stat_name[8:])
        self._summary_options.get('iprs').push(
            Range.FromExplicitRange(lower, upper))
    for stat_name in self._summary_options:
      if stat_name in ['percentile', 'iprs']:
        continue
      self._summary_options[stat_name] = stat_name in statistics_names

  def _DeserializeBin(self, i, bin_data, deserializer):
    # Copy HistogramBin on write, share the rest with the other
    # Histograms that use the same HistogramBinBoundaries.
    self._bins[i] = HistogramBin(self._bins[i].range)
    self._bins[i].Deserialize(bin_data, deserializer)

    # TODO(benjhayden): Remove after class Sample.
    if not isinstance(bin_data, list):
      return
    for sample in bin_data[1:]:
      if isinstance(sample, list):
        sample = sample[0]
      self._sample_values.append(sample)


  def _DeserializeBins(self, bins, deserializer):
    if isinstance(bins, list):
      for i, bin_data in enumerate(bins):
        self._DeserializeBin(i, bin_data, deserializer)
    else:
      for i, bin_data in bins.items():
        self._DeserializeBin(int(i), bin_data, deserializer)

  @staticmethod
  def Deserialize(data, deserializer):
    name, unit, boundaries, diagnostics, running, bins, nan_bin = data
    name = deserializer.GetObject(name)
    boundaries = HistogramBinBoundaries.FromDict(
        deserializer.GetObject(boundaries))
    hist = Histogram(name, unit, boundaries)

    hist._diagnostics.DeserializeAdd(diagnostics, deserializer)

    desc = hist.diagnostics.get(reserved_infos.DESCRIPTION.name)
    if desc:
      hist.description = list(desc)[0]

    hist._DeserializeStatistics()

    if running:
      hist._running = RunningStatistics.FromDict(running)

    if bins:
      hist._DeserializeBins(bins, deserializer)

    if isinstance(nan_bin, list):
      # TODO(benjhayden): hist._nan_bin
      hist._num_nans = nan_bin[0]
      for sample in nan_bin[1:]:
        # TODO(benjhayden): class Sample
        hist._nan_diagnostic_maps.append(
            DiagnosticMap.Deserialize(sample[1:], deserializer))
    elif nan_bin:
      hist._num_nans = nan_bin

    return hist

  @staticmethod
  def FromDict(dct):
    boundaries = HistogramBinBoundaries.FromDict(dct.get('binBoundaries'))
    hist = Histogram(dct['name'], dct['unit'], boundaries)
    if 'description' in dct:
      hist._description = dct['description']
    if 'diagnostics' in dct:
      hist._diagnostics.AddDicts(dct['diagnostics'])
    if 'allBins' in dct:
      if isinstance(dct['allBins'], list):
        for i, bin_dct in enumerate(dct['allBins']):
          # Copy HistogramBin on write, share the rest with the other
          # Histograms that use the same HistogramBinBoundaries.
          hist._bins[i] = HistogramBin(hist._bins[i].range)
          hist._bins[i].FromDict(bin_dct)
      else:
        for i, bin_dct in dct['allBins'].items():
          i = int(i)
          # Check whether i is a valid index before using it as a list index.
          if i >= len(hist._bins) or i < 0:
            raise InvalidBucketError(i, bin_dct, hist._bins)
          hist._bins[i] = HistogramBin(hist._bins[i].range)
          hist._bins[i].FromDict(bin_dct)
    if 'running' in dct:
      hist._running = RunningStatistics.FromDict(dct['running'])
    if 'summaryOptions' in dct:
      if 'iprs' in dct['summaryOptions']:
        dct['summaryOptions']['iprs'] = [
            Range.FromExplicitRange(r[0], r[1])
            for r in dct['summaryOptions']['iprs']]
      hist.CustomizeSummaryOptions(dct['summaryOptions'])
    if 'maxNumSampleValues' in dct:
      hist._max_num_sample_values = dct['maxNumSampleValues']
    if 'sampleValues' in dct:
      hist._sample_values = dct['sampleValues']
    if 'numNans' in dct:
      hist._num_nans = dct['numNans']
    if 'nanDiagnostics' in dct:
      for map_dct in dct['nanDiagnostics']:
        hist._nan_diagnostic_maps.append(DiagnosticMap.FromDict(map_dct))
    return hist

  @staticmethod
  def FromProto(proto):
    if not proto.HasField('unit'):
      raise ValueError('The "unit" field is required.')
    if not proto.name:
      raise ValueError('The "name" field is required.')

    boundaries = HistogramBinBoundaries.FromProto(proto.bin_boundaries)
    unit = histogram_proto.UnitFromProto(proto.unit)

    hist = Histogram(proto.name, unit, boundaries)

    if proto.description:
      hist._description = proto.description
    if proto.HasField('diagnostics'):
      hist._diagnostics.AddProtos(proto.diagnostics.diagnostic_map)
    for i, bin_spec in proto.all_bins.items():
      i = int(i)
      # Check whether i is a valid index before using it as a list index.
      if i >= len(hist._bins) or i < 0:
        raise InvalidBucketError(i, str(bin_spec), hist._bins)
      hist._bins[i] = HistogramBin(hist._bins[i].range)
      hist._bins[i].FromProto(bin_spec)
    if proto.HasField('running'):
      hist._running = RunningStatistics.FromProto(proto.running)
    if proto.HasField('summary_options'):
      options_dict = {
          'avg': proto.summary_options.avg,
          'geometricMean': proto.summary_options.geometric_mean,
          'std': proto.summary_options.std,
          'count': proto.summary_options.count,
          'sum': proto.summary_options.sum,
          'min': proto.summary_options.min,
          'max': proto.summary_options.max,
          'nans': proto.summary_options.nans,
          'percentile': proto.summary_options.percentile,
      }
      hist.CustomizeSummaryOptions(options_dict)
    if proto.max_num_sample_values:
      hist._max_num_sample_values = proto.max_num_sample_values
    if proto.sample_values:
      hist._sample_values = proto.sample_values
    if proto.num_nans:
      hist._num_nans = proto.num_nans

    # TODO: NaN diagnostics.

    return hist

  @property
  def num_values(self):
    if self._running is None:
      return 0
    return self._running.count

  @property
  def num_nans(self):
    return self._num_nans

  @property
  def average(self):
    if self._running is None:
      return None
    return self._running.mean

  @property
  def standard_deviation(self):
    if self._running is None:
      return None
    return self._running.stddev

  @property
  def geometric_mean(self):
    if self._running is None:
      return 0
    return self._running.geometric_mean

  @property
  def sum(self):
    if self._running is None:
      return 0
    return self._running.sum

  def GetApproximatePercentile(self, percent):
    if percent < 0 or percent > 1:
      raise ValueError('percent must be in [0,1]')
    if self.num_values == 0:
      return 0

    if len(self._bins) == 1:
      sorted_sample_values = list(self._sample_values)
      sorted_sample_values.sort()
      return sorted_sample_values[
          int((len(sorted_sample_values) - 1) * percent)]

    values_to_skip = math.floor((self.num_values - 1) * percent)
    for hbin in self._bins:
      values_to_skip -= hbin.count
      if values_to_skip >= 0:
        continue
      if hbin.range.min == -JS_MAX_VALUE:
        return hbin.range.max
      if hbin.range.max == JS_MAX_VALUE:
        return hbin.range.min
      return hbin.range.center
    return self._bins[len(self._bins) - 1].range.min

  def _ResampleMean(self, percent):
    if percent <= 0 or percent >= 1:
      raise ValueError('percent must be in (0,1)')
    filtered_samples = [
        x for x in self._sample_values if isinstance(x, (float, int))
    ]
    if not filtered_samples:
      return [0, 0]

    sample_count = len(filtered_samples)
    iterations = DEFAULT_ITERATION_FOR_BOOTSTRAP_RESAMPLING
    if sample_count == 1:
      return [filtered_samples[0], filtered_samples[0]]
    if len(self._sample_means) != iterations:
      self._sample_means = []
      for _ in range(0, iterations):
        temp_sum = 0
        for _ in range(0, sample_count):
          temp_sum += random.choice(filtered_samples)
        self._sample_means.append(temp_sum / sample_count)
      self._sample_means.sort()

    return [
        self._sample_means[int(
            math.floor((iterations - 1) * (0.5 - percent / 2)))],
        self._sample_means[int(
            math.ceil((iterations - 1) * (0.5 + percent / 2)))],
    ]

  def GetBinIndexForValue(self, value):
    index = FindHighIndexInSortedArray(
        self._bins, lambda b: (-1 if (value < b.range.max) else 1))
    if 0 <= index < len(self._bins):
      return index
    return len(self._bins) - 1

  def GetBinForValue(self, value):
    return self._bins[self.GetBinIndexForValue(value)]

  def AddSample(self, value, diagnostic_map=None):
    if (diagnostic_map is not None and
        not isinstance(diagnostic_map, DiagnosticMap)):
      diagnostic_map = DiagnosticMap(diagnostic_map)

    if not isinstance(value, numbers.Number) or math.isnan(value):
      self._num_nans += 1
      if diagnostic_map:
        UniformlySampleStream(self._nan_diagnostic_maps, self.num_nans,
                              diagnostic_map, MAX_DIAGNOSTIC_MAPS)
    else:
      if self._running is None:
        self._running = RunningStatistics()
      self._running.Add(value)

      bin_index = self.GetBinIndexForValue(value)
      hbin = self._bins[bin_index]
      if hbin.count == 0:
        hbin = HistogramBin(hbin.range)
        self._bins[bin_index] = hbin
      hbin.AddSample(value)
      if diagnostic_map:
        hbin.AddDiagnosticMap(diagnostic_map)

    UniformlySampleStream(self._sample_values, self.num_values + self.num_nans,
                          value, self.max_num_sample_values)

  def CanAddHistogram(self, other):
    if self.unit != other.unit:
      return False
    return self._bin_boundaries_dict == other._bin_boundaries_dict

  def AddHistogram(self, other):
    if not self.CanAddHistogram(other):
      raise ValueError('Merging incompatible Histograms')

    MergeSampledStreams(
        self.sample_values, self.num_values,
        other.sample_values, other.num_values,
        (self.max_num_sample_values + other.max_num_sample_values) / 2)
    self._num_nans += other._num_nans

    if other.running is not None:
      if self.running is None:
        self._running = RunningStatistics()
      self._running = self._running.Merge(other.running)

    for i, hbin in enumerate(other.bins):
      mybin = self._bins[i]
      if mybin.count == 0:
        self._bins[i] = mybin = HistogramBin(mybin.range)
      mybin.AddBin(hbin)

    self.diagnostics.Merge(other.diagnostics)

  def CustomizeSummaryOptions(self, options):
    for key, value in options.items():
      self._summary_options[key] = value

  def Clone(self):
    return Histogram.FromDict(self.AsDict())

  def CloneEmpty(self):
    return Histogram(self.name, self.unit, HistogramBinBoundaries.FromDict(
        self._bin_boundaries_dict))

  @property
  def statistics_names(self):
    names = set()
    for stat_name, option in self._summary_options.items():
      if stat_name == 'percentile':
        for pctile in option:
          names.add('pct_' + PercentToString(pctile))
      elif stat_name == 'iprs':
        for rang in option:
          names.add('ipr_' + PercentToString(rang.min, True) +
                    '_' + PercentToString(rang.max, True))
      elif stat_name == 'ci':
        for ci in option:
          ps = PercentToString(ci)
          names.add('ci_' + ps + '_lower')
          names.add('ci_' + ps + '_upper')
          names.add('ci_' + ps)
      elif option:
        names.add(stat_name)
    return names

  def GetStatisticScalar(self, stat_name):
    if stat_name == 'avg':
      if self.average is None:
        return None
      return Scalar(self.unit, self.average)

    if stat_name == 'std':
      if self.standard_deviation is None:
        return None
      return Scalar(self.unit, self.standard_deviation)

    if stat_name == 'geometricMean':
      if self.geometric_mean is None:
        return None
      return Scalar(self.unit, self.geometric_mean)

    if stat_name in ['min', 'max', 'sum']:
      if self._running is None:
        self._running = RunningStatistics()
      return Scalar(self.unit, getattr(self._running, stat_name))

    if stat_name == 'nans':
      return Scalar('count_smallerIsBetter', self.num_nans)

    if stat_name == 'count':
      return Scalar('count_smallerIsBetter', self.num_values)

    if stat_name.startswith('pct_'):
      if self.num_values == 0:
        return None
      percent = PercentFromString(stat_name[4:])
      percentile = self.GetApproximatePercentile(percent)
      return Scalar(self.unit, percentile)

    if stat_name.startswith('ci_'):
      if self.num_values == 0:
        return None
      ci = PercentFromString(stat_name[3:6])
      [ci_lower, ci_upper] = self._ResampleMean(ci)
      if stat_name.endswith('lower'):
        return Scalar(self.unit, ci_lower)
      if stat_name.endswith('upper'):
        return Scalar(self.unit, ci_upper)
      return Scalar(self.unit, ci_upper - ci_lower)

    if stat_name.startswith('ipr_'):
      lower = PercentFromString(stat_name[4:7])
      upper = PercentFromString(stat_name[8:])
      if lower >= upper:
        raise ValueError('Invalid inter-percentile range: ' + stat_name)
      return Scalar(self.unit, (self.GetApproximatePercentile(upper) -
                                self.GetApproximatePercentile(lower)))

    return None

  @property
  def statistics_scalars(self):
    results = {}
    for name in self.statistics_names:
      scalar = self.GetStatisticScalar(name)
      if scalar:
        results[name] = scalar
    return results

  def Serialize(self, serializer):
    nan_bin = self.num_nans
    if self.nan_diagnostic_maps:
      nan_bin = [nan_bin] + [
          [None] + dm.Serialize(serializer) for dm in self.nan_diagnostic_maps]
    self.diagnostics[reserved_infos.STATISTICS_NAMES.name] = \
      generic_set.GenericSet(sorted(self.statistics_names))
    self.diagnostics[reserved_infos.DESCRIPTION.name] = \
      generic_set.GenericSet([self._description])
    return [
        serializer.GetOrAllocateId(self.name),
        self.unit.replace(
            '_biggerIsBetter', '+').replace('_smallerIsBetter', '-'),
        serializer.GetOrAllocateId(self._bin_boundaries_dict),
        self.diagnostics.Serialize(serializer),
        self._running.AsDict() if self._running else 0,
        self._SerializeBins(serializer),
        nan_bin,
    ]

  def AsDict(self):
    dct = {'name': self.name, 'unit': self.unit}
    if self._bin_boundaries_dict is not None:
      dct['binBoundaries'] = self._bin_boundaries_dict
    if self._description:
      dct['description'] = self._description
    if len(self.diagnostics):
      dct['diagnostics'] = self.diagnostics.AsDict()
    if self.max_num_sample_values != self._GetDefaultMaxNumSampleValues():
      dct['maxNumSampleValues'] = self.max_num_sample_values
    if self.num_nans:
      dct['numNans'] = self.num_nans
    if self.nan_diagnostic_maps:
      dct['nanDiagnostics'] = [m.AsDict() for m in self.nan_diagnostic_maps]
    if self.num_values:
      dct['sampleValues'] = list(self.sample_values)
      dct['running'] = self._running.AsDict()
      dct['allBins'] = self._GetAllBinsAsDict()
      if dct['allBins'] is None:
        del dct['allBins']

    summary_options = {}
    any_overridden_summary_options = False
    for name, option in self._summary_options.items():
      if name in ('percentile', 'ci'):
        if len(option) == 0:
          continue
      elif name == 'iprs':
        if len(option) == 0:
          continue
        option = [[r.min, r.max] for r in option]
      elif name == 'ci':
        if len(option) == 0:
          continue
      elif option == DEFAULT_SUMMARY_OPTIONS[name]:
        continue
      summary_options[name] = option
      any_overridden_summary_options = True
    if any_overridden_summary_options:
      dct['summaryOptions'] = summary_options
    return dct

  def AsProto(self):
    proto = histogram_proto.Pb2().Histogram()

    proto.name = self.name
    unit = histogram_proto.ProtoFromUnit(self.unit)
    proto.unit.unit = unit.unit
    proto.unit.improvement_direction = unit.improvement_direction

    if self._bin_boundaries_dict:
      bounds = HistogramBinBoundaries.ToProto(self._bin_boundaries_dict)
      proto.bin_boundaries.first_bin_boundary = bounds.first_bin_boundary
      proto.bin_boundaries.bin_specs.extend(bounds.bin_specs)
    if self._description:
      proto.description = self._description
    if len(self.diagnostics):
      self.diagnostics.AsProto(proto.diagnostics)

    if self.max_num_sample_values != self._GetDefaultMaxNumSampleValues():
      proto.max_num_sample_values = self.max_num_sample_values
    if self.num_nans:
      proto.num_nans = self.num_nans
    if self.num_values:
      proto.sample_values.extend(list(self.sample_values))
      proto.running.CopyFrom(self._running.AsProto())
      self._GetAllBinsAsProto(proto.all_bins)

    any_overridden_summary_options = any(
        self._summary_options[k] != DEFAULT_SUMMARY_OPTIONS[k]
        for k in DEFAULT_SUMMARY_OPTIONS)

    if any_overridden_summary_options or self._summary_options['percentile']:
      # Note: iprs and ci are not supported in the proto format yet.
      # Also, we must set all options or none (could be fixed by moving to bool
      # wrappers in the proto).
      proto.summary_options.avg = self._summary_options['avg']
      proto.summary_options.geometric_mean = (
          self._summary_options['geometricMean'])
      proto.summary_options.std = self._summary_options['std']
      proto.summary_options.count = self._summary_options['count']
      proto.summary_options.sum = self._summary_options['sum']
      proto.summary_options.min = self._summary_options['min']
      proto.summary_options.max = self._summary_options['max']
      proto.summary_options.nans = self._summary_options['nans']
      proto.summary_options.percentile.extend(
          self._summary_options['percentile'])

    return proto

  def _SerializeBins(self, serializer):
    num_bins = len(self._bins)
    empty_bins = 0
    for hbin in self._bins:
      if hbin.count == 0:
        empty_bins += 1
    if empty_bins == num_bins:
      return None

    if empty_bins > (num_bins / 2):
      all_bins_dict = {}
      for i, hbin in enumerate(self._bins):
        if hbin.count > 0:
          all_bins_dict[i] = hbin.Serialize(serializer)
      return all_bins_dict

    all_bins_list = []
    for hbin in self._bins:
      all_bins_list.append(hbin.Serialize(serializer))
    return all_bins_list

  def _GetAllBinsAsDict(self):
    num_bins = len(self._bins)
    empty_bins = 0
    for hbin in self._bins:
      if hbin.count == 0:
        empty_bins += 1
    if empty_bins == num_bins:
      return None

    if empty_bins > (num_bins / 2):
      all_bins_dict = {}
      for i, hbin in enumerate(self._bins):
        if hbin.count > 0:
          all_bins_dict[i] = hbin.AsDict()
      return all_bins_dict

    all_bins_list = []
    for hbin in self._bins:
      all_bins_list.append(hbin.AsDict())
    return all_bins_list

  def _GetAllBinsAsProto(self, proto_bin_map):
    num_bins = len(self._bins)
    empty_bins = 0
    for hbin in self._bins:
      if hbin.count == 0:
        empty_bins += 1
    if empty_bins == num_bins:
      return

    for i, hbin in enumerate(self._bins):
      if hbin.count > 0:
        proto_bin_map[i].CopyFrom(hbin.AsProto())

  def _GetDefaultMaxNumSampleValues(self):
    return len(self._bins) * 10


class HistogramBinBoundaries(object):
  __slots__ = '_builder', '_range', '_bin_ranges', '_bins'

  CACHE = {}
  SLICE_TYPE_LINEAR = 0
  SLICE_TYPE_EXPONENTIAL = 1

  def __init__(self, min_bin_boundary):
    self._builder = [min_bin_boundary]
    self._range = Range()
    self._range.AddValue(min_bin_boundary)
    self._bin_ranges = None
    self._bins = None

  @property
  def range(self):
    return self._range

  @staticmethod
  def FromDict(dct):
    if dct is None:
      return HistogramBinBoundaries.SINGULAR

    cache_key = json.dumps(dct)
    if cache_key in HistogramBinBoundaries.CACHE:
      return HistogramBinBoundaries.CACHE[cache_key]

    bin_boundaries = HistogramBinBoundaries(dct[0])
    for slic in dct[1:]:
      if not isinstance(slic, list):
        bin_boundaries.AddBinBoundary(slic)
        continue
      if slic[0] == HistogramBinBoundaries.SLICE_TYPE_LINEAR:
        bin_boundaries.AddLinearBins(slic[1], slic[2])
      elif slic[0] == HistogramBinBoundaries.SLICE_TYPE_EXPONENTIAL:
        bin_boundaries.AddExponentialBins(slic[1], slic[2])
      else:
        raise ValueError('Unrecognized HistogramBinBoundaries slice type')

    bin_boundaries._BuildBins()
    HistogramBinBoundaries.CACHE[cache_key] = bin_boundaries
    return bin_boundaries

  @staticmethod
  def FromProto(proto):
    if not proto:
      return HistogramBinBoundaries.SINGULAR

    bin_boundaries = HistogramBinBoundaries(proto.first_bin_boundary)
    for spec in proto.bin_specs:
      if spec.HasField('bin_boundary'):
        bin_boundaries.AddBinBoundary(spec.bin_boundary)
      elif spec.HasField('bin_spec'):
        bin_spec = spec.bin_spec
        b = histogram_proto.Pb2().BinBoundaryDetailedSpec
        if bin_spec.boundary_type == b.LINEAR:
          bin_boundaries.AddLinearBins(
              bin_spec.maximum_bin_boundary, bin_spec.num_bin_boundaries)
        elif bin_spec.boundary_type == b.EXPONENTIAL:
          bin_boundaries.AddExponentialBins(
              bin_spec.maximum_bin_boundary, bin_spec.num_bin_boundaries)
        else:
          raise ValueError('Unrecognized HistogramBinBoundaries slice type')

    bin_boundaries._BuildBins()
    return bin_boundaries

  @staticmethod
  def ToProto(dct):
    proto = histogram_proto.Pb2().BinBoundaries()
    if dct is None:
      proto.first_bin_boundary = JS_MAX_VALUE
      return proto

    proto.first_bin_boundary = dct[0]
    for slic in dct[1:]:
      spec = proto.bin_specs.add()
      if not isinstance(slic, list):
        spec.bin_boundary = slic
        continue

      b = histogram_proto.Pb2().BinBoundaryDetailedSpec
      if slic[0] == HistogramBinBoundaries.SLICE_TYPE_LINEAR:
        spec.bin_spec.boundary_type = b.LINEAR
        spec.bin_spec.maximum_bin_boundary = slic[1]
        spec.bin_spec.num_bin_boundaries = slic[2]
      elif slic[0] == HistogramBinBoundaries.SLICE_TYPE_EXPONENTIAL:
        spec.bin_spec.boundary_type = b.EXPONENTIAL
        spec.bin_spec.maximum_bin_boundary = slic[1]
        spec.bin_spec.num_bin_boundaries = slic[2]
      else:
        raise ValueError('Unrecognized HistogramBinBoundaries slice type')

    return proto

  def AsDict(self):
    if len(self._builder) == 1 and self._builder[0] == JS_MAX_VALUE:
      return None
    return self._builder

  @staticmethod
  def CreateExponential(lower, upper, num_bins):
    bin_boundaries = HistogramBinBoundaries(lower)
    bin_boundaries.AddExponentialBins(upper, num_bins)
    bin_boundaries._BuildBins()
    return bin_boundaries

  @staticmethod
  def CreateLinear(lower, upper, num_bins):
    bin_boundaries = HistogramBinBoundaries(lower)
    bin_boundaries.AddLinearBins(upper, num_bins)
    bin_boundaries._BuildBins()
    return bin_boundaries

  def _PushBuilderSlice(self, slic):
    self._builder += [slic]

  def AddBinBoundary(self, next_max_bin_boundary):
    if next_max_bin_boundary <= self.range.max:
      raise ValueError('The added max bin boundary must be larger than ' +
                       'the current max boundary')

    self._bin_ranges = None
    self._bins = None

    self._PushBuilderSlice(next_max_bin_boundary)
    self.range.AddValue(next_max_bin_boundary)
    return self

  def AddLinearBins(self, next_max_bin_boundary, bin_count):
    if bin_count <= 0:
      raise ValueError('Bin count must be positive')
    if next_max_bin_boundary <= self.range.max:
      raise ValueError('The new max bin boundary must be greater than ' +
                       'the previous max bin boundary')

    self._bin_ranges = None
    self._bins = None

    self._PushBuilderSlice([
        HistogramBinBoundaries.SLICE_TYPE_LINEAR,
        next_max_bin_boundary, bin_count])
    self.range.AddValue(next_max_bin_boundary)
    return self

  def AddExponentialBins(self, next_max_bin_boundary, bin_count):
    if bin_count <= 0:
      raise ValueError('Bin count must be positive')
    if self.range.max <= 0:
      raise ValueError('Current max bin boundary must be positive')
    if self.range.max >= next_max_bin_boundary:
      raise ValueError('The last added max boundary must be greater than ' +
                       'the current max boundary boundary')

    self._bin_ranges = None
    self._bins = None

    self._PushBuilderSlice([
        HistogramBinBoundaries.SLICE_TYPE_EXPONENTIAL,
        next_max_bin_boundary, bin_count])
    self.range.AddValue(next_max_bin_boundary)
    return self

  @property
  def bins(self):
    if self._bins is None:
      self._BuildBins()
    return self._bins

  def _BuildBins(self):
    self._bins = [HistogramBin(r) for r in self.bin_ranges]

  @property
  def bin_ranges(self):
    if self._bin_ranges is None:
      self._BuildBinRanges()
    return self._bin_ranges

  def _BuildBinRanges(self):
    if not isinstance(self._builder[0], numbers.Number):
      raise ValueError('Invalid start of builder_')

    self._bin_ranges = []
    prev_boundary = self._builder[0]
    if prev_boundary > -JS_MAX_VALUE:
      # underflow bin
      self._bin_ranges.append(Range.FromExplicitRange(
          -JS_MAX_VALUE, prev_boundary))

    for slic in self._builder[1:]:
      if not isinstance(slic, list):
        self._bin_ranges.append(Range.FromExplicitRange(
            prev_boundary, slic))
        prev_boundary = slic
        continue

      next_max_bin_boundary = float(slic[1])
      bin_count = slic[2]
      slice_min_bin_boundary = float(prev_boundary)

      if slic[0] == self.SLICE_TYPE_LINEAR:
        bin_width = (next_max_bin_boundary - prev_boundary) / bin_count
        for i in range(1, bin_count):
          boundary = slice_min_bin_boundary + (i * bin_width)
          self._bin_ranges.append(Range.FromExplicitRange(
              prev_boundary, boundary))
          prev_boundary = boundary
      elif slic[0] == self.SLICE_TYPE_EXPONENTIAL:
        bin_exponent_width = (
            math.log(next_max_bin_boundary / prev_boundary) / bin_count)
        for i in range(1, bin_count):
          boundary = slice_min_bin_boundary * math.exp(i * bin_exponent_width)
          self._bin_ranges.append(Range.FromExplicitRange(
              prev_boundary, boundary))
          prev_boundary = boundary
      else:
        raise ValueError('Unrecognized HistogramBinBoundaries slice type')

      self._bin_ranges.append(Range.FromExplicitRange(
          prev_boundary, next_max_bin_boundary))
      prev_boundary = next_max_bin_boundary

    if prev_boundary < JS_MAX_VALUE:
      # overflow bin
      self._bin_ranges.append(Range.FromExplicitRange(
          prev_boundary, JS_MAX_VALUE))


HistogramBinBoundaries.SINGULAR = HistogramBinBoundaries(JS_MAX_VALUE)


# The JS version computes these values using tr.b.convertUnit, which is
# not implemented in Python, so we write them out here.
def _CreateMsAutoFormatBins():
  bins = [
      2000,
      5000,
      10000,
      30000,
      60000,
      120000,
      300000,
      600000,
      1800000,
      3600000,
      7200000,
      21600000,
      43200000,
      86400000,
      604800000,
      2629743840,
      31556926080
  ]

  boundaries = HistogramBinBoundaries(0).AddBinBoundary(1).AddExponentialBins(
      1e3, 3)

  for b in bins:
    boundaries.AddBinBoundary(b)

  return boundaries


DEFAULT_BOUNDARIES_FOR_UNIT = {
    'ms': HistogramBinBoundaries.CreateExponential(1e-3, 1e6, 100),
    'tsMs': HistogramBinBoundaries.CreateLinear(0, 1e10, 1000),
    'msBestFitFormat': _CreateMsAutoFormatBins(),
    'n%': HistogramBinBoundaries.CreateLinear(0, 1.0, 20),
    'sizeInBytes': HistogramBinBoundaries.CreateExponential(1, 1e12, 100),
    'bytesPerSecond': HistogramBinBoundaries.CreateExponential(1, 1e12, 100),
    'J': HistogramBinBoundaries.CreateExponential(1e-3, 1e3, 50),
    'W': HistogramBinBoundaries.CreateExponential(1e-3, 1, 50),
    'A': HistogramBinBoundaries.CreateExponential(1e-3, 1, 50),
    'Ah': HistogramBinBoundaries.CreateExponential(1e-3, 1, 50),
    'V': HistogramBinBoundaries.CreateExponential(1e-3, 1, 50),
    'Hz': HistogramBinBoundaries.CreateExponential(1e-3, 1, 50),
    'unitless': HistogramBinBoundaries.CreateExponential(1e-3, 1e3, 50),
    'count': HistogramBinBoundaries.CreateExponential(1, 1e3, 20),
    'sigma': HistogramBinBoundaries.CreateLinear(-5, 5, 50),
}
