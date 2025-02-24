# Pinpoint Errors

[TOC]

## The test ran successfully but failed to produce output. This is likely due to a problem with the test itself in this range. {#ReadValueNoFile}

It’s not uncommon for tests to be broken or flaky in the range that Pinpoint is
bisecting on. A typical pattern is a test is disabled or broken for a period of
time, fixed or re-enabled, regressions are detected since the test is now
producing data again, sheriffs initiate bisects which fail on the broken range.

### Suggested course of action:

Rerunning *may* work if the test is flaky, but isn’t expected to. If the test is
consistently failing, you have the option of specifying a patch to Pinpoint
which allows it to fix the test in the broken range. See these instructions:
HTTP\_DOCS

## The test ran successfully, but the output failed to contain any valid values. This is likely due to a problem with the test itself in this range. {#ReadValueNoValues}

It’s possible for a test to run and complete successfully, but not contain any
valid values. Some benchmarks will either produce NULL values or simply no
output, even on successful runs. Unfortunately, Pinpoint is unable to proceed in
these situations

### Suggested course of action:

File a bug on the benchmark’s owner.

## The test ran successfully, but the metric specified wasn't found in the output. This is likely due to a problem with the test itself in this range. {#ReadValueNotFound}

The test ran successfully, and Pinpoint was able to retrieve and parse the
output, but the metric Pinpoint is bisecting on wasn’t found in the output. This
can happen in a few situations:

*   The test is flaky, or broken in the range, or simply doesn’t output the
    metric by design under some conditions.
*   The metric was renamed.
*   An error in Pinpoint parsing out the values.

### Suggested course of action:

File a bug on Pinpoint.

## The test was run but failed and Pinpoint was unable to parse the exception from the logs. {#SwarmingTaskFailedNoException}

The test failed, but Pinpoint was unable to parse the output and surface the
exception. This can happen if something fails in the entrypoint script that
swarming uses to run the performance test, or a failure on Pinpoint’s part to
properly locate and parse the test’s stdout.

### Suggested course of action:

File a bug on Pinpoint for further investigation.

## The test was successfully queued in swarming, but expired. This is likely due to the bots being overloaded, dead, or misconfigured. Pinpoint will stop this job to avoid potentially overloading the bots further. {#SwarmingExpired}

Pinpoint maintains a private pool of devices that mirror the Chromium Perf
Waterfall’s setup. In some cases where there isn’t a lot of capacity, individual
jobs have run for extraordinary amounts of time, or devices are
offline/misconfigured, queued swarming tasks can expire, having never run.

### Suggested course of action:

File a bug on Pinpoint. Rerunning the job may simply contribute further to
congestion and cause other jobs to fail.

## There doesn't appear to be any bots available to run the performance test. Either all the swarming devices are offline, or they're misconfigured. {#SwarmingNoBots}

The Chromium Perf Waterfall and Pinpoint have a private set of devices, split
into 2 pools. A job initiated from the ChromePerf dashboard should match with a
similar device in Pinpoint’s pool. In this case, Pinpoint wasn’t able to find
any devices matching the configuration requested, which could indicate that
either there’s a misconfiguration or all devices are offline.

### Suggested course of action:

File a bug on Pinpoint.

## Services Pinpoint relies on appear to be down or unresponsive, indicating there may be a broader outage going on. {#TRANSIENT-ERROR-MSG}

Pinpoint relies on several external services (Swarming, Buildbucket, etc.). An
outage there will make it impossible for Pinpoint to continue.

### Suggested course of action:

File a bug on Pinpoint and wait for the outage to end before running any new
jobs. In the future, Pinpoint should automatically pause jobs in the situation
rather than fail.

## The build was reported to have completed successfully, but Pinpoint is unable to find the isolate that was produced and will be unable to run any tests against this revision. {#BuildIsolateNotFound}

Pinpoint has a set of private builders that are used to build revisions that the
main waterfall either hasn’t built, or revisions with specific requirements
(such as patches, dependant repo’s at specific revisions, etc.) that the main
waterfall wouldn’t have built under normal circumstances. On completion of the
builds, Pinpoint parses the result for the isolates it needs to run the specific
test requested. This can fail rarely, in cases like the isolate target changing
names, Pinpoint using an invalid name, or a transient error uploading the
results to Pinpoint.

### Suggested course of action:

File a bug on Pinpoint. Rerunning the job may work, but isn’t expected to.

## Encountered an INFRA\_FAILURE error while attempting to build this revision. Pinpoint will be unable to run any tests against this revision. {#BuildFailed-INFRA-FAILURE}

Pinpoint has a set of private builders that are used to build revisions that the
main waterfall either hasn’t built, or revisions with specific requirements
(such as patches, dependant repo’s at specific revisions, etc.) that the main
waterfall wouldn’t have built under normal circumstances.

INFRA\_FAILURE occurs when there’s an issue with one of Pinpoint’s builders,
e.g. a broken recipe roll.

### Suggested course of action:

File a bug on Pinpoint and wait for the outage to end before running any new
jobs. In the future, Pinpoint should automatically pause jobs in the situation
rather than fail.

## Encountered an BUILD\_FAILURE error while attempting to build this revision. Pinpoint will be unable to run any tests against this revision unless you re-run the job with a patch that fixes the error. Please see these instructions: HTTP\_PINPOINT\_DOCS {#BuildFailed-TODO}

Pinpoint has a set of private builders that are used to build revisions that the
main waterfall either hasn’t built, or revisions with specific requirements
(such as patches, dependant repo’s at specific revisions, etc.) that the main
waterfall wouldn’t have built under normal circumstances.

BUILD\_FAILURE failures occur when the build is broken in the range Pinpoint is
attempting to bisect. This can happen both on the main Chromium waterfall which
has occasional breakages, or in dependant repo’s where Pinpoint is manually
managing revisions.

### Suggested course of action:

Failure here is expected. Rerunning the job will only succeed if you specify a
patch to Pinpoint which allows it to fix the build in the broken range. See
these instructions: HTTP\_DOCS

## Encountered a TIMEOUT error while attempting to build this revision. Pinpoint will be unable to run any tests against this revision. {#BuildFailed-TIMEOUT}

Pinpoint has a set of private builders that are used to build revisions that the
main waterfall either hasn’t built, or revisions with specific requirements
(such as patches, dependant repo’s at specific revisions, etc.) that the main
waterfall wouldn’t have built under normal circumstances.

TIMEOUT failures occur when the builder hits the time limit for building a
revision. This often indicates a problem with the build in the range specified.

## Encountered a CANCELED\_EXPLICITLY error while attempting to build this revision. Pinpoint will be unable to run any tests against this revision. {#BuildFailed-CANCELED-EXPLICITLY}

Pinpoint has a set of private builders that are used to build revisions that the
main waterfall either hasn’t built, or revisions with specific requirements
(such as patches, dependant repo’s at specific revisions, etc.) that the main
waterfall wouldn’t have built under normal circumstances.

CANCELED\_EXPLICITLY failures occur when the build is explicitly cancelled by
someone. Pinpoint will intentionally stop the job in this case.

### Suggested course of action:

If this was unexpected, file a bug on Pinpoint.

## Pinpoint currently only supports the fully redirected patch URL, ie. https://chromium-review.googlesource.com/c/chromium/src/+/12345 {#BuildGerritURLInvalid}

Pinpoint currently only supports the fully redirected patch URL, ie.
https://chromium-review.googlesource.com/c/chromium/src/+/12345

### Suggested course of action:

If this is a try-job, please re-run the job with the fully redirected URL.

If this is a bisect-job, please file a bug on Pinpoint.

## Pinpoint encountered a fatal internal error and cannot continue. Please file an issue with Speed>Bisection. {#FATAL-ERROR-MSG}

Pinpoint hit an unexpected error. This is a catch-all when Pinpoint hits an
error we haven’t seen before.

### Suggested course of action:

File a bug under Speed\>Bisection so that the team can investigate.

## Pinpoint has hit its' retry limit and will terminate this job. {#RETRY-LIMIT}

Pinpoint typically retries failed actions when it can. Ie. url fetch timeouts,
transient oauth errors, etc. When it hits the retry limit, the job will stop.

### Suggested course of action:

File a bug under Speed\>Bisection so that the team can investigate.
