# Chrome Performance Dashboard Data Formats

Currently, the performance dashboard supports two formats for data upload:
- Histograms, via the `add_histograms` endpoint.
- Chart JSON, via the `add_point` endpoint.

While both formats remain supported, histograms are preferred if your tool can
produce them as an output format.

**Note:** [Telemetry](/telemetry/README.md), in particular, is due to deprecate
Chart JSON output in favor of Histograms.

## Histograms

The endpoint for new histograms
(`https://chromeperf.appspot.com/add_histograms`) accepts HTTP POST
requests. The body of the request should be a JSON encoded and gzip compressed
list of histogram dicts. See
[HistogramSet JSON Format](/docs/histogram-set-json-format.md) for details on
the format.


## Chart JSON v1

The endpoint for new points
(`https://chromeperf.appspot.com/add_point`) accepts HTTP POST
requests. With the POST request, there should be one parameter given,
called "data", the value of which is JSON which contains all of the data
being uploaded.

Example:

```javascript
{
  "master": "master.chromium.perf",
  "bot": "linux-release",
  "point_id": 123456,
  "versions": {
    "version type": "version string"
  },
  "supplemental": {
    "field name": "supplemental data string",
    "default_rev": "r_chrome_version"
  },
  "chart_data": {/*... as output by Telemetry; see below ...*/}
}
```

Fields:

 * `master` (string): Buildbot master name or top-level category for data.
 * `bot` (string): Buildbot builder name, or another string that
 represents platform type.
 * `test_suite_name` (string): A string to use in the perf dashboard test
 * path after master/bot. Can contain slashes.
 * `format_version` (string): Allows dashboard to know how to process
 the structure.
 * `versions` (dict): Maps repo name to revision. Ping dashboard admins to
   get a new revision type added; here are some available ones:
   * `chrome_version`: Version number of Chrome, like 54.0.2840.71
   * `chromium_git`: Chromium git hash
   * `chromium_commit_pos`: Chromium commit position
   * `v8_git`: v8 git hash
 * `supplemental` (dict): Unstructured key-value pairs which may be
 displayed on the dashboard. Used to describe bot hardware, OS,
 Chrome feature status, etc.
   * `default_rev`: This is used to specify which version type to show as the
      default in tooltips and x-axis. It maps to one of the keys in the
      `versions` dict, with an `r_` prepended. For example, if you want to
      set the default version to chrome_version, you'd specify
      `default-rev`: `r_chrome_version`.
 * `chart_data` (dict): The chart JSON as output by Telemetry.

### Chart data:

This contains all of the test results and any metadata that is stored with
the test.

```json
{
  "format_version": "1.0",
  "benchmark_name": "page_cycler.typical_25",
  "charts": {
    "warm_times": {
      "http://www.google.com/": {
        "type": "list_of_scalar_values",
        "values": [9, 9, 8, 9],
      },
      "http://www.yahoo.com/": {
        "type": "list_of_scalar_values",
        "values": [4, 5, 4, 4],
        "std": 0.5,
      },
      "summary": {
        "type": "list_of_scalar_values",
        "values": [13, 14, 12, 13],
        "file": "gs://..."
      },
    },
    "html_size": {
      "http://www.google.com/": {
        "type": "scalar",
        "value": 13579,
        "units": "bytes"
      }
    },
    "load_times": {
      "http://www.google.com/": {
        "type": "list_of_scalar_values",
        "value": [4.2],
        "std": 1.25,
        "units": "sec"
      }
    }
  }
}
```

Fields:

 * `charts`: [dict of string to dict] Maps a list of chart name strings
 to their data dicts.
 * `units`: [string] Units to display on the dashboard.
 * `traces`: [dict of string to dict] Maps a list of trace name strings
 to their trace dicts.
 * `type`: [string] `"scalar"`, `"list_of_scalar_values"` or `"histogram"`,
 which tells the dashboard how to interpret the rest of the fields.
   * `scalar` points require the field `"value"` [number].
   * `list_of_scalar_values` points require the field `"values"` [list of
   numbers]. The field `"std"` [number] can optionally specify the sample
   standard deviation of the values, otherwise the dashboard will compute it.
 * `improvement_direction`: [string] Either `"bigger_is_better"`, or
 `"smaller_is_better"`.
 * `summary`: A special trace name which denotes the trace in a chart which does
 not correspond to a specific page.

## Legacy Format

This format is deprecated and should not be used for new clients.

In the format described below, the value of "data" in the HTTP POST
should be a JSON encoding of a list of points to add. Each point is a
map of property names to values for that point.

Example 1:

```json
[
  {
    "master": "SenderType",
    "bot": "platform-type",
    "test": "my_test_suite/chart_name/trace_name",
    "revision": 1234,
    "value": 18.5
  }
]
```

Required fields:

 * `master` (string), `bot` (string), `test` (string): These three
 fields in combination specify a particular "test path". The master and
 bot are supposed to be the Buildbot master name and slave `perf_id`,
 respectively, but if the tests aren't being run by Buildbot, these
 can be any descriptive strings which specify the test data origin
 (note master and bot names can't contain slashes, and none of these
 can contain asterisks).
 * `revision` (int): The point ID, used to index the data point. It
 doesn't actually have to be a "revision". Should be monotonically increasing
 for data in each series.
 * `value` (float): The Y-value for this point.

Example 2 (including optional fields):

```json
[
  {
    "master": "ChromiumPerf",
    "bot": "linux-release",
    "test": "sunspider/string-unpack-code/ref",
    "revision": 33241,
    "value": "18.5",
    "error": "0.5",
    "units": "ms",
    "masterid": "master.chromium.perf",
    "buildername": "Linux Builder",
    "buildnumber": 75,
    "supplemental_columns": {
      "r_webkit_rev": "167808",
      "a_default_rev": "r_webkit_rev"
    }
  },
  {
    "master": "ChromiumPerf",
    "bot": "linux-release",
    "test": "sunspider/string-unpack-code",
    "revision": 33241,
    "value": "18.4",
    "error": "0.489",
    "units": "ms",
    "masterid": "master.chromium.perf",
    "buildername": "Linux Builder",
    "buildnumber": 75,
    "supplemental_columns": {
      "r_webkit_rev": "167808",
      "a_default_rev": "r_webkit_rev"
    }
  }
]
```

Optional fields:

 * `units` (string): The (y-axis) units for this point.
 * `error` (float): A standard error or standard deviation value.
 * `supplemental_columns`: A dictionary of other data associated with
 this point.
   * Properties starting with `r\_` are revision/version numbers.
   * Properties starting with `d\_` are extra data numbers.
   * Properties starting with `a\_` are extra metadata strings.
     * `a_default_rev`: The name of a another supplemental property key
     starting with "a_".
     * `a_stdio_uri`: Link to stdio logs for the test run.
 * `higher_is_better` (boolean). You can use this field to explicitly
 define improvement direction.

## Providing test and unit information

Sending test descriptions are supported in with Dashboard JSON v1.
Test descriptions for Telemetry tests are provided in code for the
benchmarks, and are included by Telemetry in the chart JSON output.

## Relevant code links

Implementations of code that sends data to the dashboard:

 * `chromium/build/scripts/slave/results_dashboard.py`
 * `chromiumos/src/third_party/autotest/files/tko/perf_upload/perf_uploader.py`

## Getting set up with new test results

Once you're ready to start sending data to the real perf dashboard, there
are a few more things you might want to do. Firstly, in order for the
dashboard to accept the data, the IP of the sender must be added to the
IP allowlist.

If your data is not internal-only data, you can request that it be marked
as such, again by filing an issue.

Finally, if you want to monitor your the test results, you can decide
which tests you want to be monitored, who should be receiving alerts, and
whether you want to set any special thresholds for alerting.

## Contact

In general, for questions or requests you can email
chrome-perf-dashboard-team@google.com.
