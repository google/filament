# Gsutil Continuous Integration Testing

**PREAMBLE**: This document describes the system we use for integration tests and how to use it.

## Kokoro CI

### Overview
-----

Kokoro aims to be our primary CI system. For internal Googlers, its documentation is at [go/gsutil-ci](http://go/gsutil-ci) and the design document for integrating with Kokoro is at [go/gsutil-test-matrix](http://go/gsutil-test-matrix).

These tests launch every time a PR is submitted by a trusted author (Googler in the GoogleCloudPlatform org), or when a trusted author manually invokes tests on an external contributer's PR by applying the `kokoro:run` label.

The build configs found in this repository under the `gsutil/test/ci/kokoro` directory and the job configs found internally at [go/gsutil-kokoro-piper](http://go/gsutil-kokoro-piper) define how Kokoro will run our tests, with what scripts, and with which VMs.

### Test Matrix
-----

We currently support Gsutil on Windows, Mac, and Linux, using Python versoins 3.5, 3.6, 3.7, and any future 3.x versions. Additionally, integration tests need to be run separately for each API, both XML and JSON.

Each of these 12+ combinations of `(OS / Python version / API)` is run on a separate VM managed by Kokoro, all running in parallel.

These tests launch every time a PR is submitted by a trusted author (Googler in the GoogleCloudPlatform org), or when a trusted author manually invokes tests on an external contributer's PR by applying the `kokoro:run` label.

### Test Scripts
-----

Linux and Mac share the same bash script in `gsutil/test/ci/kokoro/run_integ_tests.sh`. Windows targets a `gsutil/test/ci/kokoro/windows/run_integ_tests.bat` which initializes environment variables and passes them to a powershell script which runs the tests in parallel.

### Test Results
-----

Unfortunately, test results and logs aren't yet available for the public.

