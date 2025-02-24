<!-- Copyright 2015 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->
Bisect on catapult
=================

The purpose of this library is to house the logic used by the [bisect
recipe](https://code.google.com/p/chromium/codesearch#chromium/build/scripts/slave/recipes/bisect.py)
to improve its stability, testability and mathematical soundness beyond what the
recipes subsystem currently allows.

Secondary goals are:

 * Simplify code sharing with the related [Telemetry](/telemetry/README.md) and [Performance Dashboard](/dashboard/README.md) projects.
 * Eventually move the bisect director role outside of buildbot/recipes and
   into its own standalone application.
