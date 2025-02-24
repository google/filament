This document describes the Google Analytics metrics reported by results.html.

Measures are recorded by the performance web API, and are visible in the User
Timing track in the devtools Performance timeline.

Instant events are recorded using console.timestamp(), and are visible as orange
ticks at the top of the devtools Performance timeline.

Both measures and instant events are recorded as Events in Google Analytics.
[Access these metrics here](https://analytics.google.com/analytics/web/#embed/report-home/a98760012w145165698p149871853/) if you have been granted access.

 * histogram-set-controls
   * `alpha` measures response latency of changing the statistical significance
     threshold, alpha.
   * `hideOverviewCharts` measures response latency of hiding all overview
     charts.
   * `referenceColumn` measures response latency of changing the reference
     column.
   * `search` measures response latency of changing the search query to filter
     rows.
   * `showAll` measures response latency of toggling showing all rows versus
     source Histograms only.
   * `showOverviewCharts` measures response latency of showing all overview
     charts.
   * `statistic` measures response latency of changing the statistic that is
     displayed in histogram-set-table-cells.
 * HistogramSetLocation
   * `onPopState` measures response latency of the browser back button.
   * `pushState` measures latency of serializing the view state and pushing it
     to the HTML5 history API. This happens automatically whenever any part of
     the ViewState is updated.
 * histogram-set-table
   * `columnCount` instant event contains the number of columns, recorded when the
     table is built.
   * `nameColumnConstrained` instant event recorded when the name column width
     is constrained.
   * `nameColumnUnconstrained` instant event recorded when the name column width
     is unconstrained.
   * `rootRowCount` instant event contains the number of root rows, recorded
     whenever it changes or the table is built.
   * `rowCollapsed` instant event recorded whenever a row is collapsed.
   * `rowExpanded` instant event recorded whenever a row is expanded.
   * `selectHistogramNames` instant event recorded whenever a breakdown related
     histogram name link is clicked.
   * `sortColumn` instant event recorded whenever the user changes the sort
     column.
 * histogram-set-table-cell
   * `close` instant event recorded when the cell is closed.
   * `open` instant event recorded when the cell is opened.
 * histogram-set-table-name-cell
   * `closeHistograms` instant event recorded when the user clicks the button to
     close all histogram-set-table-cells in the row.
   * `hideOverview` instant event recorded when the user clicks the button to
     hide the overview line charts for the row.
   * `openHistograms` instant event recorded when the user clicks the button to
     open all histogram-set-table-cells in the row.
   * `showOverview` instant event recorded when the user clicks the button to
     show the overview line charts for the row.
 * histogram-set-view
   * `build` measures latency to find source Histograms, collect parameters,
     configure the controls and build the table. Does not include parsing
     Histograms from json.
   * `sourceHistograms` measures latency to find source Histograms in the
     relationship graphical model.
   * `collectParameters` measures latency to collect display labels, statistic
     names, and possible groupings.
   * `export{Raw,Merged}{CSV,JSON}` measures latency to download a CSV/JSON file
     of raw/merged Histograms.
 * histogram-span
   * `brushBins` instant event recorded when the user finishes brushing bins.
   * `clearBrushedBins` instant event recorded when the user clears brushed
     bins.
   * `mergeSampleDiagnostics` measures latency of displaying the table of merged
     sample diagnostics.
   * `splitSampleDiagnostics` measures latency of displaying the table of
     unmerged sample diagnostics.
 * HistogramParameterCollector
   * `maxSampleCount` instant event records maximum Histogram.numValues
