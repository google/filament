# Debugging User Issues With Upload Completion Tokens

Users of the performance dashboard, especially newer users,
may report issues that their uploaded data doesn't show up
on the dashboard, despite Waterfall indicating a succesful
upload. This can occur, for example, if the user's
data is in the wrong format or some process randomly fails during
the upload.

To debug these issues, we use upload completion tokens. A unique token is
always output into the build logs of all performance builders. This token
can be used to search the GCP Server Logs of the performance dashboard to
find which downstream upload task failed and how.

## Locating Upload Completion Tokens in the Build Logs

1. Ask the user which builder was responsible for the upload of their
performance tests. A full list of builders can be found
[here](https://ci.chromium.org/p/chrome/g/chrome.perf/builders).
2. Click on the matching builder link
(e.g. https://ci.chromium.org/p/chrome/g/chrome.perf/builders/ci/Android%20Nexus5%20Perf).
3. Click on the most recent successful build's "Build #".
4. Take note of the time the build was run at.
5. Under "Steps & Logs", open the stdout for the Step prefixed by
"performance_test_suite ...".
6. Search for "Upload completion token created. Token id:". There can be
multiple matches. Picking any single token should be enough for most cases.

## Locating Upload Completion Tokens in the Server Logs

1. Once we have a token id, go to
https://pantheon.corp.google.com/logs/query?project=chromeperf.
2. Set the custom time range to reflect the build's execution time.
3. Set the Query to the following and run:
	```"{upload completion token id}" AND "state: FAILED"```
	. We'll be able to see the failed task(s) after search is complete.
4. The failed tasks can then be further debugged from here and the user should be
notified on how to fix their tests, once the error has been identified.
