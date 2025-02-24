Using Trace Viewer Casually
==================================
 * [Embedding-Trace-Viewer](https://github.com/catapult-project/catapult/blob/master/tracing/docs/embedding-trace-viewer.md) the trace-viewer in your own app.
 * How to [extend and customize](https://github.com/catapult-project/catapult/blob/master/tracing/docs/extending-and-customizing-trace-viewer.md) the trace-viewer to suit your domain

Making Traces
=============
 * [Trace Event Format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit?usp=sharing) if you want to generate traces yourself
 * [py-trace-event](https://github.com/natduca/py_trace_event) for generating traces from python
 * [Chrome's trace_event.h](http://src.chromium.org/chrome/trunk/src/base/debug/trace_event.h) if you're in Chrome's ecosystem
 * [ftrace](https://www.kernel.org/doc/Documentation/trace/ftrace.txt) for generating traces on Linux

Note: trace-viewer supports custom trace file formats. Just [add an importer](https://github.com/catapult-project/catapult/blob/master/tracing/docs/extending-and-customizing-trace-viewer.md) to trace viewer for your favorite file format.

Contributing New Stuff
======================
 * Join our Google Groups: [trace-viewer](https://groups.google.com/forum/#!forum/trace-viewer), [trace-viewer-bugs](https://groups.google.com/forum/#!forum/trace-viewer-bugs)
 * Learn how to start: [Contributing](https://github.com/catapult-project/catapult/blob/master/CONTRIBUTING.md)
 * Read the [Trace Viewer style guide](https://docs.google.com/document/d/1MMOfywou2Oaho4jOttUk-ZSJcHVd5G5BTsD48rPrBtQ/edit)
 * Pick a feature from the [tracing wish list](https://docs.google.com/a/chromium.org/document/d/1T1UJHIgImSEPSugCt2TFrkNsraBFITPHpYFGDJStePc/preview).
 * Familiarize yourself with the [Trace-Viewer's-Internals](https://github.com/catapult-project/catapult/blob/master/tracing/docs/trace-viewer-internals.md).
