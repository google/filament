
<!-- Copyright 2015 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->
![Trace Viewer Logo](https://raw.githubusercontent.com/catapult-project/catapult/master/tracing/images/trace-viewer-circle-blue.png)

Trace-Viewer is the javascript frontend for Chrome [about:tracing](http://dev.chromium.org/developers/how-tos/trace-event-profiling-tool) and [Android
systrace](http://developer.android.com/tools/help/systrace.html).

It provides rich analysis and visualization capabilities for many types of trace
files. Its particularly good at viewing linux kernel traces (aka [ftrace](https://www.kernel.org/doc/Documentation/trace/ftrace.txt)) and Chrome's
[trace_event format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview). Trace viewer can be [embedded](https://github.com/catapult-project/catapult/blob/master/tracing/docs/embedding-trace-viewer.md) as a component in your own code, or used from a plain checkout to turn trace files into standalone, emailable HTML files from the commandline:

```
$CATAPULT/tracing/bin/trace2html my_trace.json --output=my_trace.html && open my_trace.html
```

Its easy to [extend trace viewer](https://github.com/catapult-project/catapult/blob/master/tracing/docs/extending-and-customizing-trace-viewer.md) to support your favorite trace format, or add domain specific visualizations to the UI to simplify drilling down into complex data.

Contributing, quick version
===
We welcome contributions! To hack on this code.

There are two type of tests.

### In the browser

Run http server `$CATAPULT/bin/run_dev_server`. In any browser, navigate to `http://localhost:8003/`

**Unit tests**| **Descripton**
--- | ---
All tests | http://localhost:8003/tests.html
All tests with short format | http://localhost:8003/tracing/tests.html?shortFormat
An individual test suite(such as ui/foo_test.js) | http://localhost:8003/tests.html?testSuiteName=ui.foo
Tests named foo| http://localhost:8003/tests.html?testFilterString=foo

### On command

**Unit tests**| **Description**
--- | ---
All python tests | `$CATAPULT/tracing/bin/run_py_tests`
All tracing tests in d8 environment | `$CATAPULT/tracing/bin/run_vinn_tests`
All tracing tests in devserver environment | `$CATAPULT/tracing/bin/run_devserver_tests`
All tests | `$CATAPULT/tracing/bin/run_tests`

Make sure tests pass before sending us changelist. **We use Gerrit for codereview**. For more details, esp on Gerrit, [read our contributing guide](https://github.com/catapult-project/catapult/blob/master/CONTRIBUTING.md) or check out the [Getting Started guide](https://github.com/catapult-project/catapult/blob/master/tracing/docs/getting-started.md).

Contact Us
===
Join our Google Group:
* [tracing@chromium.org](https://groups.google.com/a/chromium.org/forum/#!forum/tracing)
