# Telemetry Tests

Telemetry has few suites of unit tests:
* telemetry_unittests in catapult: [catapult/telemetry/bin/run_tests](https://github.com/catapult-project/catapult/blob/master/telemetry/bin/run_tests). This suite verifies the core framework functions against the stable browser.
* telemetry_unittests in chromium repo: [src/tools/perf/run_telemetry_tests](https://cs.chromium.org/chromium/src/tools/perf/run_telemetry_tests). This suite verifies the core framework functions against your browser built in chromium environment.
* telemetry_perf_unittests: [src/tools/perf/run_tests](https://cs.chromium.org/chromium/src/tools/perf/run_tests). This suite verifies that Chromium benchmarks are using the framework function properly.

These are functional tests that do not depend on performance.

# Triaging failures
* **Is there a native stack?** Since these tests interact with a lot of recorded real world content, they unintentionally end up serving as integration tests which frequently uncover Chromium crashes. If you see a native crash stack (after a `TabCrashException` or `BrowserGoneException`), this is guaranteed to be a browser issue. Usually scanning the change log for patches that touch files that show up in the stack will point to the culprit to revert.
* **Is there a Python stack?** If there's a Python-only exception, it is very likely, but not guaranteed to be a Telemetry breakage. Look for Telemetry changes in the range for a culprit.
* **Is there a timeout?** These could go either way and are tricky to diagnose, move on to local diagnostics.
*
# Running the tests locally
* Preparing your devices for testing: [go/telemetry-device-setup](http://go/telemetry-device-setup) (Googler only for now, sorry!)
* [Authenticate into Cloud Storage.](https://sites.google.com/a/chromium.org/dev/developers/telemetry/upload_to_cloud_storage)
* Run test via:
     `$ catapult/telemetry/run_tests <test> --browser=<browser> --chrome-root=<path to chromium src/ dir>`
Where `<test>` can be: `BrowserTest.testForegroundTab` or `BrowserTest` as a “wildcard” by matching the sub string `"BrowserTest"`.
Where `<path to chromium src/ dir>` is the full path including the `src/` at the end and `<browser>` can be: `release`, `android-chrome-shell`, `list` (for a full list)

# Disabling tests

Tests should generally only be disabled for flakiness. Consistent failures should be diagnosed and the culprit reverted.

The `@decorators.Disabled` and `@decorators.Enabled` decorators may be added above any test to enable or disable it. They optionally accept a list of platforms, os versions or browser types. Examples:
```
from telemetry import decorators
...
```
* `@decorators.Disabled('all')`    # Disabled everywhere
* `@decorators.Enabled('mac')`     # Only runs on mac
* `@decorators.Disabled('xp')`     # Run everywhere except windows xp
* `@decorators.Disabled('debug')`  # Run everywhere except debug builds
