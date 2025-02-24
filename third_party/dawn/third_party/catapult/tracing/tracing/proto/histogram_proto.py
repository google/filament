# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os.path
import sys

main_file = getattr(sys.modules['__main__'], '__file__', None)
if main_file and os.path.basename(main_file) == 'print_python_deps.py':
  # The module is being loaded by the app that calculates Python dependencies.
  # Don't try to load histogram_pb2, as that module exists in some environments
  # and is missing in some other environments. Loading it would cause the
  # dependency list to become nondeterministic.
  HAS_PROTO = False
else:
  try:
    # Note: from tracing.proto import histogram_pb2 would make more sense here,
    # but unfortunately protoc does not generate __init__.py files if you
    # specify an out package (at least for the gn proto_library rule).
    import histogram_pb2
    HAS_PROTO = True
  except ImportError as e:
    try:
      # crbug/1234919
      # Catapult put the generated histogram_pb2.py in the same source folder,
      # while the others (e.g., webrtc) put it in output path. By default we
      # try to import from the sys.path. Here allows to try import from the
      # source folder as well.
      # TODO(wenbinzhang): Clean up import paths to work consistently.
      from . import histogram_pb2
      HAS_PROTO = True
    except ImportError:
      HAS_PROTO = False


def _EnsureProto():
  """Ensures histogram_pb.py is in the PYTHONPATH.

  If the assert fails here, it means your script doesn't ensure histogram_pb2.py
  is generated and is in the PYTHONPATH. To fix this, depend on the GN rule
  in BUILD.gn and ensure the script gets the out/Whatever/pyproto dir in its
  PYTHONPATH (for instance by making your script take a --out-dir=out/Whatever
  flag).
  """
  assert HAS_PROTO, ('Tried to use histogram protos, but missing '
                     'histogram_pb2.py. Try cd tracing/proto && make.')


def Pb2():
  """Resolves the histogram proto stub.

  Where you would use histogram_pb2.X, instead do histogram_proto.Pb2().X.
  """
  _EnsureProto()
  return histogram_pb2


if HAS_PROTO:
  PROTO_UNIT_MAP = {
      histogram_pb2.MS: 'ms',
      histogram_pb2.MS_BEST_FIT_FORMAT: 'msBestFitFormat',
      histogram_pb2.TS_MS: 'tsMs',
      histogram_pb2.N_PERCENT: 'n%',
      histogram_pb2.SIZE_IN_BYTES: 'sizeInBytes',
      histogram_pb2.BYTES_PER_SECOND: 'bytesPerSecond',
      histogram_pb2.J: 'J',
      histogram_pb2.W: 'W',
      histogram_pb2.A: 'A',
      histogram_pb2.V: 'V',
      histogram_pb2.HERTZ: 'Hz',
      histogram_pb2.UNITLESS: 'unitless',
      histogram_pb2.COUNT: 'count',
      histogram_pb2.SIGMA: 'sigma',
  }
  UNIT_PROTO_MAP = {v: k for k, v in PROTO_UNIT_MAP.items()}

  PROTO_IMPROVEMENT_DIRECTION_MAP = {
      histogram_pb2.BIGGER_IS_BETTER: 'biggerIsBetter',
      histogram_pb2.SMALLER_IS_BETTER: 'smallerIsBetter',
  }
  IMPROVEMENT_DIRECTION_PROTO_MAP = {
      v: k for k, v in PROTO_IMPROVEMENT_DIRECTION_MAP.items()
  }


def UnitFromProto(proto_unit):
  _EnsureProto()
  direction = proto_unit.improvement_direction
  unit = PROTO_UNIT_MAP[proto_unit.unit]
  if direction and direction != histogram_pb2.NOT_SPECIFIED:
    unit += '_' + PROTO_IMPROVEMENT_DIRECTION_MAP[direction]

  return unit


def ProtoFromUnit(unit):
  _EnsureProto()

  parts = unit.split('_')

  assert unit
  assert 0 < len(parts) <= 2, ('expected <unit>_(bigger|smaller)IsBetter' +
                               str(parts))

  proto_unit = histogram_pb2.UnitAndDirection()
  proto_unit.unit = UNIT_PROTO_MAP[parts[0]]
  if len(parts) > 1:
    proto_unit.improvement_direction = IMPROVEMENT_DIRECTION_PROTO_MAP[parts[1]]

  return proto_unit
