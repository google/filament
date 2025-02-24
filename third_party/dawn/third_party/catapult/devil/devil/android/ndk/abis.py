# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Android NDK ABIs.

https://developer.android.com/ndk/guides/abis

These constants can be compared against the value of
devil.android.DeviceUtils.product_cpu_abi.
"""

ARM = 'armeabi-v7a'
ARM_64 = 'arm64-v8a'
X86 = 'x86'
X86_64 = 'x86_64'
