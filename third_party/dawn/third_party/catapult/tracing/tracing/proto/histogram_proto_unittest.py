# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest
import sys
from six.moves import reload_module

# Some packages, such as protobuf, clobber the google
# namespace package. This prevents that.
# crbug/1233198
if 'google' in sys.modules:
  reload_module(sys.modules['google'])

from tracing.proto import histogram_proto# pylint: disable=wrong-import-position


class HistogramProtoUnittest(unittest.TestCase):
  def testUnitFromProto(self):
    proto = histogram_proto.Pb2()

    proto_unit = proto.UnitAndDirection()
    proto_unit.unit = proto.N_PERCENT
    proto_unit.improvement_direction = proto.BIGGER_IS_BETTER

    proto_unit2 = proto.UnitAndDirection()
    proto_unit2.unit = proto.BYTES_PER_SECOND
    proto_unit2.improvement_direction = proto.SMALLER_IS_BETTER

    proto_unit3 = proto.UnitAndDirection()
    proto_unit3.unit = proto.SIGMA

    self.assertEqual('n%_biggerIsBetter',
                     histogram_proto.UnitFromProto(proto_unit))
    self.assertEqual('bytesPerSecond_smallerIsBetter',
                     histogram_proto.UnitFromProto(proto_unit2))
    self.assertEqual('sigma',
                     histogram_proto.UnitFromProto(proto_unit3))

  def testProtoFromUnit(self):
    proto = histogram_proto.Pb2()

    unit1 = histogram_proto.ProtoFromUnit('count_biggerIsBetter')
    unit2 = histogram_proto.ProtoFromUnit('Hz_smallerIsBetter')
    unit3 = histogram_proto.ProtoFromUnit('unitless')

    self.assertEqual(unit1.unit, proto.COUNT)
    self.assertEqual(unit1.improvement_direction, proto.BIGGER_IS_BETTER)
    self.assertEqual(unit2.unit, proto.HERTZ)
    self.assertEqual(unit2.improvement_direction, proto.SMALLER_IS_BETTER)
    self.assertEqual(unit3.unit, proto.UNITLESS)
