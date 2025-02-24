<!-- Copyright 2016 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->

# Metrics Results UI

This document assumes familiarity with the concepts introduced in
[how-to-write-metrics](/docs/how-to-write-metrics.md).

Metrics produce Histograms, which are serialized into an HTML file named
`results.html` by default. Upon opening this HTML file in Chrome, after all of
the Histograms are loaded, the `histogram-set-table` processes them and displays
many names, numbers, and controls.

This complex UI can be overwhelming initially, but understanding how to use it
can pay off in deep performance insights.

## The Table

We'll skip the top two rows of controls for now because they modify what
information is displayed in the table, which we need to understand first.

### The Name Column

The first column always contains Names. Usually, these are Histogram names,
which are the strings that metrics pass to Histogram constructors. However, as
we'll see in "The Groupby-picker", if you group by attributes of Histograms
besides their name, like the "story name", then this first Name column will
contain the name of the grouping of Histograms, such as the story name.

![The first, left-most column is the Name column.
](/docs/images/metrics-results-ui-name.png)

### The Value Column(s)

The other columns contain Histogram values.
One of the other attributes of Histograms is their `displayLabel`. If these
Histograms were produced by Telemetry, then you can pass `--results-label` to
Telemetry's `run_benchmark` command to set a label for all of the Histograms
produced by that benchmark run. Otherwise, displayLabel will default to the
benchmark name plus the time that the benchmark was run.
There is a separate value column for each distinct displayLabel value, which is
displayed in the header cell.

![The other columns contain values splayed by label.
](/docs/images/metrics-results-ui-value.png)

### Grouping Tree

Histograms can be grouped into a tree configured by the groupby-picker,
described below.
Non-leaf node Histograms are merged from the Histograms that are grouped under
them.
Histograms that are grouped together are only mergeable if they share the same
units and bin boundaries.

![Rows are grouped and nested to form a tree.
](/docs/images/metrics-results-ui-grouping.png)

If a non-leaf node Histogram is merged from unmergeable Histograms, then the
table will display that non-leaf node Histogram as the string "(unmergeable)".

![Attempts to merge unmergeable Histograms display "(unmergeable)".
](/docs/images/metrics-results-ui-unmergeable.png)

### Sorting

Click the up/down pointing triangles in the table header row in order to sort
the table by that column.
Sorting the Name column sorts alphabetically.
Sorting Value columns sorts numerically by Histogram average.

![Columns are sortable.
](/docs/images/metrics-results-ui-sorting.png)

### The Overview Line Charts

Click a blue line chart icon in the Name column to display the overview line
charts in that row. These line charts graph Histogram averages. The x-axes of
these charts differ:

#### Overview Line Charts in the Name Column

The x-axis of the overview line chart in the Name column corresponds to the
value columns.

![Overview line chart comparing columns.
](/docs/images/metrics-results-ui-name-overview.png)

#### Overview Line Charts in Value Columns

The x-axis of the overview line chart in the Value columns correspond to the
Histograms in the Grouping Tree that were merged to produce the Histogram in
that table cell.

![Overview line chart comparing subrows.
](/docs/images/metrics-results-ui-value-overview.png)

### The Show Histogram Button

By default, table cells contain only averages of the Histograms in those cells.

Click the bar chart icon in table cells to display more information about the
Histogram:
 * Histogram bin count bar chart (if the Histogram has only a single bin, then
   it will be a box chart),
 * summary statistics as configured by the metric,
 * Up to 3 groups of diagnostics may be available in tabs:
    * Metadata
    * Sample diagnostics
    * Other diagnostics added to Histograms by metrics

![Diagnostics tabs
](/docs/images/metrics-results-ui-diagnostics-tabs.png)

Metadata may contain
 * Information about the device that produced the results
 * Telemetry context
 * Source control revisions
 * Links to logs and traces
These metadata diagnostics are added to Histograms by telemetry or other
processes outside of metrics.

Metrics can add diagnostics either to samples within Histograms or to Histograms
themselves.

Click and drag the mouse around the bar chart in order to display sample
diagnostics for the selected Histogram bins.

![Show full Histogram data for a cell.
](/docs/images/metrics-results-ui-show-histogram.png)

Sample diagnostics may include Breakdown diagnostics, which indicate relative
sizes of component metrics for the selected samples.

![Breakdowns](/docs/images/how-to-write-metrics-breakdown.png)

When the name of and entries in a Breakdown diagnostic align with a
RelatedNameMap diagnostic on the parent Histogram, the table rows become
clickable links to the related Histograms.
For more about summation metrics, see [Diagnostic
Metrics](https://chromium.googlesource.com/chromium/src/+/HEAD/docs/speed/diagnostic_metrics.md#Summation-Diagnostics).

## The Groupby-picker

The `groupby-picker` sits just above the table and allows you to control how
Histograms are grouped into the Grouping Tree.

Grouping keys include
 * Histogram name
 * Benchmark name
 * Story name
 * Storyset repetition
 * Story repetition
 * Legacy TIR label for TBMv1 benchmarks
 * Story grouping keys

You can enable or disable grouping keys by clicking the checkboxes, and re-order
them by clicking the left and right arrows.

![The groupby-picker configures how Histograms are grouped.
](/docs/images/metrics-results-ui-groupby-picker.png)

## The Top Controls

Finally, the row of controls at the top of the screen contains several different
small control elements. Let's start at the left.

### The Search box

Type a [regex](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Regular_Expressions)
in the Search box to filter Histograms out of the displayed HistogramSet whose
name do not match the regex.

For example, the v8 metric produces Histograms whose names end with either
"duration" or "count". The "duration" Histograms cannot be merged with the
"count" histograms. If you group by any other grouping key besides "name" first,
then the table will try to merge some "count" Histograms with some "duration"
Histograms. The two different kinds of Histograms have different units, so they
cannot be merged, so the table will display "(unmergeable)". In order to make
the table ignore the "count" Histograms so that it can merge all of the
"duration" Histograms together, you can filter out the "count" Histograms by
entering "duration" in the Search box.

![Filter Histograms by searching.
](/docs/images/metrics-results-ui-search.png)

### The Total Overview Chart Button

Toggle the Overview Line Charts for all visible rows at once by clicking the
blue line chart button to the right of the Search box.

![Show overview line charts for all visible rows.
](/docs/images/metrics-results-ui-total-overview.png)

### The Reference Column Selector

The Reference Column Selector is displayed when there are multiple Value columns
in order to allow you to select one. When one Value column is selected as a
reference, then all of the other Value columns will update to display the delta
between that column and the reference Value column.

For example, if there are two Value columns "A" and "B", and you select "A" in
the Reference Column Selector, then the values displayed in the "A" Value column
will remain the same, but the values displayed in the "B" Value column will
change to display the differences between the values in the "B" column and the
corresponding values in the "A" column.

![Select a reference column.
](/docs/images/metrics-results-ui-reference-column.png)

When you open a histogram in a non-reference Value column, in addition to the
statistics produced by the Histogram such as "avg", "stddev", "min", "max", etc.
the Histogram statistic table will also display statistics about the delta
between that Histogram and the corresponding reference Histogram. Delta
statistics include
 * abs&#916;avg: absolute delta between Histogram averages,
 * %&#916;avg: percent delta average,
 * z-score: (aka standard score) the number of standard deviations between the
   Histogram average and the reference Histogram average,
 * p-value: the probability that the Histograms are NOT significantly different,
 * U-statistic: a number produced and consumed by the [Mann-Whitney U hypothesis
   test](https://en.wikipedia.org/wiki/Mann%E2%80%93Whitney_U_test)

#### Significance

In order to decide whether a regression is significant enough that you should
avoid committing a change, you need to know how to interpret the delta
statistics. To begin, there are actually two different kinds of "significance",
and you need to evaluate both independently.

##### Statistical Significance

Basically, low p-value = more confidence that two Histograms are statistically
significantly different.

For more detail, the p-value is the probability under the null hypothesis
of obtaining results more extreme than your actual results. The null hypothesis
in this case states that
 * The sample distributions in the two Histograms (reference and non-reference)
   could have been taken from the same underlying true population of
   measurements (be they memory, power, time, etc.).

If the p-value is low enough, this is sufficient statistical evidence for us to
reject the null hypothesis. If the p-value is high, then we fail to reject the
null hypothesis since there is not enough statistical evidence to provide
confidence that the samples are from different populations. In context,
rejecting the null hypothesis means that the two histograms were actually taken
from different populations: a pre-regression state and a post-regression state.

"Low" is subjective, but results.html defaults to "less than &#945;=0.01", but
you could argue that &#945; should be as high as 0.1 or as low as 0.001 for your
metric.

In order to make it easy for you to quickly visually find statistically
significantly different results, an emoji is displayed in cells in non-reference
columns when a reference column is selected.
 * A green grinning face means that the current Histogram is statistically
   significantly (p-value < 0.01) better than the reference column.
 * A red frowning face means that the current Histogram is statistically significantly
   (p-value < 0.01) worse than the reference column.
 * A neutral face means that the current Histogram is not statistically significantly
   different (p-value >= 0.01) from the reference column, or else the metric
   did not indicate whether up is better or down is better.

For more detail on how p-values are computed, see the [Mann-Whitney U
test](https://en.wikipedia.org/wiki/Mann%E2%80%93Whitney_U_test).

##### Significant Magnitude

It is possible for a regression to be statistically significant, but small
enough that you shouldn't worry about it.
When sample distributions have very low deviation, then even tiny changes can be
detected statistically.

Results.html cannot yet automatically determine whether the magnitude of a
regression or improvement is significant.

There are currently a few options for manual ways to decide whether the
magnitude of a regression is significant:
 * Ask somebody who is familiar with the metric that produced the results.
 * If the benchmark is monitored, you can look at the history of the metric on
   [the dashboard](https://chromeperf.appspot.com/report). You might not
   be able to directly compare the numbers on the dashboard with your numbers
   since they are recorded on specific machines in a lab environment, but you
   can see how stable the metric has been over time: Does the metric routinely
   fluctuate by 5%?
 * Look at the %&#916;avg. A 1% regression is probably not significant, but a
   10% regression probably is.

### The Significance Threshold Slider

This slider allows you to adjust the threshold of statistical significance when
comparing results in one column against the reference column. Raising the
threshold causes more changes to be considered significant, and display green
grinning or red frowning emoji. Lowering the threshold causes fewer changes to
be considered significant, and display a neutral emoji.

![Slide the significance threshold
](/docs/images/metrics-results-ui-significance-threshold-slider.png)

### The Summary Statistic Selector

This selector controls which statistics are displayed in the table. You can
select from the any of the statistics that any Histogram was configured to
produce by the metric that produced it.

When a reference column is selected, you can also select from any of several
delta statistics: absΔavg, %Δavg, z-score, absΔstd, %Δstd, p-value, U.
These delta statistics will only be displayed in non-reference columns; avg or
std will be displayed in the reference column.

![Select a summary statistic.
](/docs/images/metrics-results-ui-summary-statistic-selector.png)

Among these statistics, percentiles, interpercentile ranges and confidence
intervals are listed as pct_XXX, ipr_XXX_YYY and CI_XXX_[lower/upper].
value of XXX can be between 000 to 100 which covers range of 0.00 to 1.00.
  * pct_XXX: The sample value at X.XX percentile (e.g. pct_095 for 0.95
  percentile).
  * ipr_XXX_YYY: The inter percentile range between X.XX and Y.YY percentile.
  * CI_XXX_lower: The lower bound of X.XX confidence interval of mean.
  * CI_XXX_upper: The upper bound of X.XX confidence interval of mean.
  * CI_XXX: The difference between the upper bound and lower bound of X.XX
  confidence interval.

### The Download CSV Button

Clicking this button downloads a CSV file that can be imported into a
spreadsheet app such as Google Sheets.

After the header row, there will be one row for each of the leaf Histograms in
the Grouping Tree. For example, if you group only by Histogram name, then there
will be as many value rows as there are distinct Histogram names in the
displayed HistogramSet.

The columns will contain
 * Histogram name and units
 * summary statistics as configured by the metrics that produced the Histograms
   (avg, stddev, min, max, etc.),
 * HistogramGrouping values.

![Download leaf Histograms as CSV.
](/docs/images/metrics-results-ui-download-csv.png)

### The Show All Checkbox

By default, this checkbox is unchecked so that the table only displays
Histograms that are source nodes in the metric graphical model.
Check this checkbox in order to show all Histograms, including those that are
not source nodes in the metric graphical model.

![Show All Histograms checkbox.
](/docs/images/metrics-results-ui-show-all.png)
