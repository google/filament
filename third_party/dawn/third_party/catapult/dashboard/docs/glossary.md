# Perf Dashboard Project Glossary

## Data organization

*Test path*: A string which serves as the identifier of a single data
series.  This is a slash-separated sequence of strings.

*Test suite name*: A top-level test name, representing a collection of
related data series whose results are output together. This is often a
Telemetry benchmark.

*Chart name*: This usually refers to the 4th part of the test path,
which often comes from the "chart name" in Telemetry data.

*Trace name*: In the dashboard code, this used to refer to the 5th
part of a test path, which used to be called "trace name" in Telemetry.
This should be discouraged now, since "trace" has other meanings in
other projects.

*Data series*: A sequence of (x, y) pairs, plotted as a line chart.

*Test*: In the perf dashboard code, test often refers to a single
data series and associated data, since for each data series there is a
corresponding Test entity in datastore.

## Perf sheriff rotations

*Sheriff*: Also known as a perf sheriff rotation, this is a group or
person who is interested in regressions in a particular set of tests.

*Monitored*: A test is monitored if there's a sheriff rotation that will
receive alerts for regressions in that test.

*Anomaly*: In the dashboard code, an anomaly refers to a step up or step
down in test results.

*Change point*: A point in a data series where there is some change. In
the case of performance tests, we're generally concerned with step-like
increases and decreases.

*To triage*: To assign a bug number to an alert, or mark it as "invalid" or
"ignored".

## Chromium continuous integration infrastructure

*Master*: A Buildbot master, also called a "waterfall". For example, the
Chromium Perf waterfall is also known as chromium.perf or ChromiumPerf.

*Bot*: A Buildbot builder; each platform type will have a different builder
name.

*Reference (ref) build*: On the Chromium Perf waterfall, tests are run
on both ToT Chromium and on an older build of Chromium. This is useful
because if the ref build results change along with the ToT results,
we can conclude that this was not caused by a change in Chrome.
