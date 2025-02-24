<!-- Copyright 2016 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->

# Devil: Persistent Device List

## What is it?

A persistent device list that stores all expected devices between builds. It
is used by non-swarmed bots to detect any missing/extra devices attached to
them.

This will be no longer needed when all bots are switched over to swarming.

## Bots

The list is usually located at:

  - `~/.android/known_devices.json`.

Look at recipes listed below in order to find more up to date location.

## Local Runs

The persistent device list is unnecessary for local runs. It is only used on the
bots that upload data to the perf dashboard.

## Where it is used

The persistent device list is used in the
[chromium_android](https://source.chromium.org/chromium/chromium/tools/build/+/HEAD:recipes/recipe_modules/chromium_android/api.py;l=50;drc=fd928820620dff8989e853accc54b1d61657f236)
recipe module, and consumed by the
[device_status.py](https://source.chromium.org/chromium/chromium/src/+/HEAD:third_party/catapult/devil/devil/android/tools/device_status.py;l=230;drc=1e5bef4469199e4daba5d8fd885966112f8a45d5)
script among others.
