# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Pure Python implementation of the Kolmogorov-Smirnov test.

This code is adapted from SciPy:
  https://github.com/scipy/scipy/blob/master/scipy/stats/stats.py
Which is provided under a BSD-style license.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import bisect
import math


def KolmogorovSmirnov(x, y):
  """Computes the 2-sample Kolmogorov-Smirnov test.

  This is a two-sided test for the null hypothesis that 2 independent samples
  are drawn from the same continuous distribution.
  """
  n1 = len(x)
  n2 = len(y)
  x = sorted(x)
  y = sorted(y)
  data_all = x + y
  cdf1 = [bisect.bisect(x, value) / float(n1) for value in data_all]
  cdf2 = [bisect.bisect(y, value) / float(n2) for value in data_all]
  d = max(math.fabs(a - b) for a, b in zip(cdf1, cdf2))
  # Note: d absolute not signed distance
  en = math.sqrt(n1 * n2 / float(n1 + n2))
  return _Kolmogorov((en + 0.12 + 0.11 / en) * d)


def _Kolmogorov(y):
  """Survival function of the Kolmogorov-Smirnov two-sided test for large N.

  https://github.com/scipy/scipy/blob/master/scipy/special/cephes/kolmogorov.c
  """
  if y < 1.1e-16:
    return 1.0

  x = -2.0 * y * y
  sign = 1.0
  p = 0.0
  r = 1.0

  while True:
    t = math.exp(x * r * r)
    p += sign * t
    if t == 0.0:
      break
    r += 1.0
    sign = -sign
    if t / p <= 1.1e-16:
      break

  return p + p
