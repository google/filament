<!-- Copyright 2017 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->
atrace_helper is an optional binary which can be pushed onto the device running
systrace in order to enrich the traces with further details (memory, I/O, etc).

Which problem is it solving?
---------------------------
Some nice-to-have details are not present in the systrace, specifically:
 - Memory snapshots of running processes (PSS/RSS).
 - Periodic snapshotting of processes and thread names.
 - File paths for filesystem events (today they report only inode numbers).

How is it solving it?
---------------------
atrace_helper is a small userspace binary which is meant to be pushed on the
device and run together with atrace by a dedicated tracing agent. When stopped,
the helper produces a JSON file which contains all the relevant details
(see --help). The JSON file is consumed by the TraceViewer importers and the
extra details are merged into the final model.

Build instructions
------------------
Building the binary requires the Android NDK to be installed. See
[Android NDK page](https://developer.android.com/ndk).
Once installed the binary can be just built as follows:
`$(NDK_HOME)/ndk-build`
The binary will be built in `libs/armeabi-v7a/`
