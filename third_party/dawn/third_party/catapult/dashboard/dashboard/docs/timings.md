This document describes the Google Analytics metrics reported by
the chromeperf dashboard.

Measures are recorded by the performance web API, and are visible in the User
Timing track in the devtools Performance timeline.

Instant events are recorded using console.timestamp(), and are visible as orange
ticks at the top of the devtools Performance timeline.

Both measures and instant events are recorded as Events in Google Analytics.

* test-picker
  * `getSuiteItems` measures response latency of getting the list of menu items
    for test suites.
  * `onDropdownSelect` measures latency of updating the dropdown menu when the
    user makes a selection.
  * `updateTestSuiteDescription` measures latency to update the description of
    the test suite.
  * `updateBotMenu` measures latency of updating the bot menu, which includes
    retrieving the bot items (getBotItems) and updating the add button state
    (updateAddButtonState).
  * `getBotItems` measures response latency of retrieving the bot menu items for
    the selected test suite.
  * `updateAddButtonState` measures the latency of updating the add button state
    by checking that the selected series can be added.
  * `subtestsXhr` measures the latency of the xhr for subtests.
  * `getCurrentSelectedPathUpTo` measures the latency of getting the current
    selected path.
  * `updateSubtestMenus` measures the latency of updating the subtest menus.
* report-page
  * `addChart` measures response latency of adding a new chart to the dashboard.
  * `onChartClosed` measures response latency of closing a chart.
  * `onRevisionRangeChanged` measures the latency of changing the revision range
    of one chart and updating the rest of the charts.
* chart-legend
  * `onCheckboxClicked` measures the latency of checking a checkbox in the chart
    legend and updating the graph.
