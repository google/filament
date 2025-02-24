# Pages and Endpoints

## Main public web pages and their query parameters.

**/**: View recent regressions and improvements.
 - *days*: Number of days to show anomalies (optional).
 - *sheriff*: Sheriff to show anomalies for (optional)
 - *num\_changes*: The number of improvements/regressions to list.

**/alerts**: View current outstanding alerts
 - *sheriff*: A sheriff rotation name, defaults to Chromium Perf Sheriff.
 - *triaged*: Whether to include recent already-triaged alerts.
 - *improvements*: Whether to include improvement alerts.
 - *sortby*: A field in the alerts table to sort rows by.
 - *sortdirection*: Direction to sort, either "up" or "down".

**/report**: Browse graphs and compare charts across platforms.
 - *sid*: A stored combination set of tests and graphs to view.
 - *masters*: Comma-separated list of master names
 - *bots*: Comma-separated list of bot names.
 - *tests*: Comma-separated list of test paths starting from benchmark name.
 - *rev*: Revision number (optional). Should correspond to the "Point ID" in
   the tooltip for a data point.
 - *num\_points*: Number of points to plot (optional). If *rev* is specified,
   it is the number of points around *rev* to show. If *start\_rev* and
   *end\_rev* are specified, it is not used. If no revision parameters are
   specified, it is the number of most recent points to show.
 - *start\_rev*: Starting revision number (optional). Must be used together with
   *end\_rev*.
 - *end\_rev*: Ending revision number (optional). Must be used together with
   *start\_rev*.
 - *checked*: Series to check. Could be "core" (important + ref) or "all".

**/group\_report**: View graphs for a set of alerts
 - *bug\_id*: Bug ID to view alerts for.
 - *rev*: Chromium commit position to view alerts for.
 - *keys*: Comma-separated list URL-safe keys, each represents one alert

**/debug\_alert**: Experiment with the alerting function, or diagnose why and when an alert would occur at some place.
 - *test\_path*: Full test path (Master/bot/benchmark/...) to get points for.
 - *rev*: A revision to center the graph on.
 - *num\_before*: Number of points to fetch before rev.
 - *num\_after*: Number of points to fetch starting from rev.
 - *config*: JSON containing custom thresholds parameters.

**/graph\_csv**: Download data points from a chart in CSV format.
- *test\_path*: Full test path (Master/bot/benchmark...) to get data points for.
  This is shown in the tooltip for each data point.
- *rev*: Revision number (optional). Should correspond to the "Point ID" in
  the tooltip for a data point. If none is specified, will return the most
  recent points.
- *num_points*: The number of data points to return. Defaults to 500.
- *attributes*: Specific attributes from `Row` entity to return (will just
  return the revision and value if not specified).

**/bisect\_stats**: View bisect job success rate stats.

**/set\_warning\_message**: Set a warning message about outages and planned maintenance.

## Administrative pages

See `Admin` menu when logged in as an administrator.
