<!-- Copyright 2016 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->

# How to Write Metrics

Timeline-Based Measurement v2 is a system for computing metrics from traces.

A TBM2 metric is a Javascript function that takes a trace Model and produces
Histograms.


## Coding Practices

Please follow the [Catapult Javascript style guide](/docs/style-guide.md) so
that the TBM2 maintainers can refactor your metric when we need to update the TBM2
API.

Please write a unit test for your metric.

If your metric computes information from the trace that may be of general use to
other metrics or the trace viewer UI, then the TBM2 maintainers may ask for your
help to generalize your innovation into a part of the Trace Model such as the
[UserModel](/tracing/tracing/model/user_model/user_model.html) or
[ModelHelpers](/tracing/tracing/model/helpers/chrome_browser_helper.html).

Use the dev server to develop and debug your metric.

 * Run `./bin/run_dev_server`
 * Navigate to
   [http://localhost:8003/tracing_examples/trace_viewer.html](http://localhost:8003/tracing_examples/trace_viewer.html).
 * Open a trace that your metric can be computed from.
 * Open the Metrics side panel on the right.
 * Select your metric from the drop-down.
 * Inspect the results and change your metric if necessary.
 * Open different traces to explore corner cases in your metric.


## Trace Model

Trace logs are JSON files produced by tracing systems in Chrome, Android, linux
perf, BattOr, etc. The trace model is an object-level representation of events
parsed from a trace log. The trace model contains Javascript objects
representing

 * OS [processes](/tracing/tracing/model/process.html),
   [threads](/tracing/tracing/model/thread.html),
 * utilities for finding special processes and threads such as
   [ChromeBrowserHelper](/tracing/tracing/model/helpers/chrome_browser_helper.html),
 * synchronous [ThreadSlices](/tracing/tracing/model/thread_slice.html)
   and asynchronous [AsyncSlices](/tracing/tracing/model/async_slice.html),
 * [snapshots](/tracing/tracing/model/object_snapshot.html) of object state as it changes throughout time,
 * [RPCs](/tracing/tracing/model/flow_event.html),
 * [FrameBlameContexts](/tracing/tracing/extras/chrome/blame_context/blame_context.html),
 * battery [power samples](/tracing/tracing/model/power_sample.html),
 * synthetic higher-level abstractions representing complex sets of
   events such as
   [UserExpectations](/tracing/tracing/model/user_model/user_expectation.html),
 * and [more](/tracing/tracing/model/model.html)!


## Histograms

A [Histogram](/tracing/tracing/value/histogram.html) is basically a common
[histogram](https://en.wikipedia.org/wiki/Histogram), but with a few extra bells
and whistles that are particularly useful for TBM2 metrics.

 * Specify units of samples and improvement direction with
   [Unit](/tracing/tracing/value/unit.html)
 * JSON serialization with asDict()/fromDict()
 * Build custom bin boundaries with HistogramBinBoundaries
 * Underflow and overflow bins for samples outside of the range of the central
   bins
 * Compute statistics such as average, stddev, sum, and percentiles
 * Customize which statistics are serialized with customizeSummaryOptions()
 * Count non-numeric samples
 * Store a random subset of sample values
 * getDifferenceSignificance() computes whether two histograms are significantly
   different with a Mann-Whitney U hypothesis test
 * addHistogram() merges two Histograms with the same units and bin boundaries

But the most complex special feature of Histograms is their Diagnostics.


## Diagnostics

When a metric significantly regresses, you then need to diagnose why it
regressed. Diagnostics are pieces of information that metrics attach to
Histograms in order help you diagnose regressions. Diagnostics may be associated
either with the entire Histogram directly, or with a particular sample.

Attach a Diagnostic to a Histogram:

```javascript
histogram.diagnostics.set('name', diagnostic)
// or
histograms.addHistogram(histogram, {name: diagnostic})
```

Attach a Diagnostic to all Histograms in a HistogramSet:

```javascript
histograms.addSharedDiagnosticToAllHistograms(name, diagnostic);
```

Attach a Diagnostic to a sample:

```javascript
histogram.addSample(number, {name: diagnostic})
```

### General Diagnostics

 * [GenericSet](/tracing/tracing/value/diagnostics/generic_set.html): This can contain
   any data that can be serialized and deserialized using JSON.stringify() and
   JSON.parse(), including numbers, strings, Arrays, and dictionaries (simple
   Objects). It will be visualized using
   [generic-object-view](/tracing/tracing/ui/analysis/generic_object_view.html),
   which is quite smart about displaying tabular data using tables, URLs using
   HTML anchor tags, pretty-printing, recursive data structures, and more.

   ![](/docs/images/how-to-write-metrics-generic.png)

 * [Breakdown](/tracing/tracing/value/diagnostics/breakdown.html):
   Structurally, these are Maps from strings to numbers. Conceptually, they
   describe what fraction of a whole (either a Histogram or a sample) is due to
   some sort of category - either a category of event, CPU sample, memory
   consumer, whathaveyou. Visually, they are a stacked bar chart with a single
   bar, which is spiritually a pie chart, but less misleading.

   ![](/docs/images/how-to-write-metrics-breakdown.png)

 * [RelatedEventSet](/tracing/tracing/value/diagnostics/related_event_set.html):
   This is a Set of references to Events in the trace model. Visually, they
   are displayed as HTML links which, when clicked in the metrics-side-panel,
   select the referenced Events in the trace viewer's timeline view. When
   clicked in results.html, they currently do nothing, but should eventually
   open the trace that contains the events and select them.

   ![](/docs/images/how-to-write-metrics-related-event-set.png)

 * [DateRange](/tracing/tracing/value/diagnostics/date_range.html):
   This is a Range of Dates. It cannot be empty, but the minDate could be the
   same as the maxDate. Telemetry automatically adds 2 shared DateRanges to all
   results: 'benchmarkStart' and 'traceStart'.

   ![](/docs/images/how-to-write-metrics-date-range.png)

### Histogram Relationship Diagnostics

 * [RelatedNameMap](/tracing/tracing/value/diagnostics/related_name_map.html):
   This maps from short keys to Histogram name. These are correlated with
   Breakdowns. They are visualized as HTML links in Breakdowns.


### Other Diagnostics

 * [Scalar](/tracing/tracing/value/diagnostics/scalar.html):
   Metrics must not use this, since it is incapable of being merged. It is
   mentioned here for completeness. It wraps a Scalar, which is just a
   unitted number. This is only to allow Histograms in other parts of the trace
   viewer to display number sample diagnostics more intelligently than
   GenericSet can. If a metric wants to display number sample diagnostics
   intelligently, then it should use Breakdown and RelatedNameMap.


### Reserved Names

Metrics may not use the following names for Histogram-level Diagnostics.

 * alertGrouping is a GenericSet of strings. Each string is a grouping key that
   will be used by the dashboard auto-triage system to [group
   alerts](https://goto.google.com/chromeperf-alert-grouping-dd) together.
 * angleRevisions is a GenericSet of strings containing
   [Angle](https://chromium.googlesource.com/angle/angle/) git hashes.
 * architectures is a GenericSet of strings containing [CPU
   architectures](https://en.wikipedia.org/wiki/List_of_CPU_architectures).
 * benchmarks is a GenericSet of strings containing Telemetry benchmark names.
 * benchmarkStart is a DateRange containing timestamps of Telemetry benchmark
   runs.
 * bots is a GenericSet of strings containing bot hostnames.
 * bugComponents is a GenericSet of strings containing [Monorail
   components](https://bugs.chromium.org/p/chromium/issues/advsearch).
 * builds is a GenericSet of numbers containing Chromium build numbers.
 * catapultRevisions is a GenericSet of strings containing
   [Catapult](https://github.com/catapult-project/catapult) git hashes.
 * chromiumCommitPositions is a GenericSet of numbers containing Chromium commit
   positions.
 * chromiumRevisions is a GenericSet of strings containing
   [Chromium](https://chromium.googlesource.com/chromium/src/) git hashes.
 * documentationUrls is a GenericSet of strings containing the urls to the
   documentation of the benchmarks/metrics.
 * gpus is a GenericSet of objects containing metadata about GPUs.
 * labels is a GenericSet of strings containing [user-defined
   labels](https://github.com/catapult-project/catapult/blob/b0f1e24d4686b3ce46667c0124a186e414fbd006/telemetry/telemetry/internal/results/results_options.py#L82)
   for Telemetry results.
 * logUrls is a GenericSet of strings containing URLs pointing to human-readable
   logs.
 * masters is a GenericSet of strings containing bot master hostnames.
 * memoryAmounts is a GenericSet of numbers containing the total amount of RAM
   in the device that recorded the Chromium trace.
 * osNames is a GenericSet of strings containing names of OSs like 'linux' and
   'mac'.
 * osVersions is a GenericSet of strings containing OS versions.
 * owners is a GenericSet of strings containing email addresses of owners of
   Telemetry benchmarks.
 * productVersions is a GenericSet of strings containing Chromium product
   versions like '60.0.9999.0'.
 * relatedNames is a GenericSet of strings containing names of related
   Histograms.
 * skiaRevisions is a GenericSet of strings containing
   [Skia](https://chromium.googlesource.com/skia/) git hashes.
 * stories is a GenericSet of strings containing Telemetry story names.
 * storysetRepeats is a GenericSet of numbers containing Telemetry storyset
   repetition counters.
 * storyTags is a GenericSet of strings containing Telemetry story tags.
 * tagmap maps from Telemetry story tags to story names.
 * traceStart is a DateRange containing timestamps of Chromium traces.
 * traceUrls is a GenericSet of strings containing URLs pointing to Chromium
   traces.
 * v8CommitPositions is a GenericSet of numbers containing V8 commit positions.
 * v8Revisions is a GenericSet of strings containing
   [V8](https://chromium.googlesource.com/v8/v8/) git hashes.
 * webrtcRevisions is a GenericSet of strings containing
   [webrtc](https://chromium.googlesource.com/external/webrtc/) git hashes.

Consumers of metrics results generally cannot rely on the presence or absence of
any of these metadata diagnostics.
Consumers can rely on the presence of Telemetry metadata for results produced by
Telemetry.
If present, they may contain a single value or multiple values.

If uploading to the ChromePerf dashboard, the following diagnostics are required
to be shared by all Histograms, and must all contain exactly one value:

 * bots
 * benchmarks
 * chromiumCommitPositions
 * masters


## Alert grouping
Setting alert groupings for your histograms allows the dashboard to be
[smarter](https://goto.google.com/chromeperf-alert-grouping-dd) about how to
group alerts together. In order to define alert groupings for your metrics,
first add it to `tracing/tracing/value/diagnostics/alert_groups.html`. Then you
can set these alert groupings either by calling `setAlertGrouping` on your
histgoram:

```javascript
my_hist.setAlertGrouping([
  tr.v.d.ALERT_GROUPS.LOADING_PAINT,
  tr.v.d.ALERT_GROUPS.LOADING_INTERACTIVITY]);
```

or by defining it when you create your histogram:

```javascript
const my_hist = tr.v.Histogram.create('foo', unitlessNumber, [], {
  alertGrouping: [
    tr.v.d.ALERT_GROUPS.LOADING_PAINT,
    tr.v.d.ALERT_GROUPS.LOADING_INTERACTIVITY,
  ]});
```

## Consumers of Histograms

Histograms are consumed by

 * [histogram-set-table](/tracing/tracing/value/ui/histogram_set_table.html) in
   both results.html and the Metrics side panel in trace viewer,
 * the [dashboard](https://chromeperf.appspot.com) indirectly via their statistics.

Currently, telemetry discards Histograms and Diagnostics, and only passes their
statistics scalars to the dashboard. Histograms and their Diagnostics will be
passed directly to the dashboard early 2017.

Metrics can control which statistics are uploaded to the dashboard by passing a
dictionary to customizeSummaryOptions() to enable or disable statistics. The
default options are as follows:

 * `avg` (average/mean): true
 * `geometricMean`: false
 * `std` (standard deviation): true
 * `count` (number of samples): true
 * `sum`: true
 * `min`: true
 * `max`: true
 * `nans` (number of non-numeric samples): false
 * `percentile`: []
   * Unlike the other options which are booleans, percentile is an array of
     numbers between 0 and 1. In order to upload the median, for example, a
     metric would call `histogram.customizeSummaryOptions({percentile: [0.5]})`.


## How histogram-set-table Uses Merging

The histogram-set-table element uses the predefined
[HistogramGroupings](/tracing/tracing/value/histogram_set.html), along with the
merging capabilities of Histograms, to allow dynamic, hierarchical organization
of histograms:

* Predefined HistogramGroupings specify how to find the benchmark, story, etc.
  that produced the Histogram.
* After loading histograms, histogram-set-table computes categories to be
  displayed by the groupby picker at the top of the UI:
  * Categories are HistogramGroupings that have more than one value across
    all histograms in the HistogramSet.
  * Instead of having one category for all story grouping keys, each grouping
    individual grouping key may be listed as a category. For example, in Page
    Cycler v2 benchmarks, the "cache_temperature" grouping key would be
    displayed as a category.
* Choosing groups builds a hierarchy of histograms that is filled in by merging
  histograms from the bottom up. Expanding the rows of histogram-set-table, any
  leaf nodes are histograms that were loaded, and their ancestors are computed by
  merging.
* histogram-set-table uses the "label" HistogramGrouping to define the columns
  of the table.
