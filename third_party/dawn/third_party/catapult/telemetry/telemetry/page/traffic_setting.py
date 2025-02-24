# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import collections

_Configs = collections.namedtuple(
    '_Configs', ('download_bandwidth_kbps,'
                 'upload_bandwidth_kbps,'
                 'round_trip_latency_ms'))

# These presets are copied from devtool's:
# https://chromium.googlesource.com/chromium/src/+/3ff085d04100b20b8e33b6d72f9505d944046c6a/third_party/WebKit/Source/devtools/front_end/components/NetworkConditionsSelector.js#36
# (note: devtools' presets are expressed as Bytes/sec.

NONE = 'none'
GPRS = 'GPRS'
REGULAR_2G = 'Regular-2G'
GOOD_2G = 'Good-2G'
REGULAR_3G = 'Regular-3G'
GOOD_3G = 'Good-3G'
REGULAR_4G = 'Regular-4G'
DSL = 'DSL'
WIFI = 'WiFi'


NETWORK_CONFIGS = {
    NONE: _Configs(0, 0, 0),
    GPRS: _Configs(50, 20, 500),
    REGULAR_2G: _Configs(250, 50, 300),
    GOOD_2G: _Configs(450, 150, 150),
    REGULAR_3G: _Configs(750, 250, 100),
    GOOD_3G: _Configs(1.5 * 1024, 750, 40),
    REGULAR_4G: _Configs(4 * 1024, 3 * 1024, 20),
    DSL: _Configs(2 * 1024, 1 * 1024, 5),
    WIFI: _Configs(30 * 1024, 15 * 1024, 2),
}
