# Adding Timing Metadata

## listing_meta.json files

`listing_meta.json` files are SEMI AUTO-GENERATED.

The raw data may be edited manually, to add entries or change timing values.

The list of tests in this file is **not** guaranteed to stay up to date.
Use the generated `gen/*_variant_list*.json` if you need a complete list.

The `subcaseMS` values are estimates. They can be set to 0 or omitted if for some reason
you can't estimate the time (or there's an existing test with a long name and
slow subcases that would result in query strings that are too long).
It's OK if the number is estimated too high.

These entries are estimates for the amount of time that subcases take to run,
and are used as inputs into the WPT tooling to attempt to portion out tests into
approximately same-sized chunks. High estimates are OK, they just may generate
more chunks than necessary.

To check for missing or 0 entries, run
`tools/validate --print-metadata-warnings src/webgpu`
and look at the resulting warnings.

### Performance

Note this data is typically captured by developers using higher-end
computers, so typical test machines might execute more slowly. For this
reason, the WPT chunking should be configured to generate chunks much shorter
than 5 seconds (a typical default time limit in WPT test executors) so they
should still execute in under 5 seconds on lower-end computers.

## Problem

When renaming or removing tests from the CTS you will see an error like this
when running `npm test` or `npm run standalone`:

```
ERROR: Non-existent tests found in listing_meta.json. Please update:
  webgpu:api,operation,adapter,requestAdapter:old_test_that_got_renamed:*
```

## Solution

This means there is a stale line in `src/webgpu/listing_meta.json` that needs
to be deleted, or updated to match the rename that you did.

## Problem

You run `tools/validate --print-metadata-warnings src/webgpu`
and want to fix the warnings.

## Solution 1 (manual, best for one-off updates of simple tests)

If you're developing new tests and need to update this file, it is sometimes
easiest to do so manually. Run your tests under your usual development workflow
and see how long they take. In the standalone web runner `npm start`, the total
time for a test case is reported on the right-hand side when the case logs are
expanded.

Record the average time per *subcase* across all cases of the test (you may need
to compute this) into the `listing_meta.json` file.

## Solution 2 (semi-automated)

There exists tooling in the CTS repo for generating appropriate estimates for
these values, though they do require some manual intervention. The rest of this
doc will be a walkthrough of running these tools.

Timing data can be captured in bulk and "merged" into this file using
the `merge_listing_times` tool. This is
This is useful when a large number of tests
change or otherwise a lot of tests need to be updated, but it also automates the
manual steps above.

The tool can also be used without any inputs to reformat `listing_meta.json`.
Please read the help message of `merge_listing_times` for more information.

### Websocket Logger

The first tool that needs to be run is `websocket-logger`, which receives data
on a WebSocket channel at `localhost:59497` to capture timing data when CTS is run. This
should be run in a separate process/terminal, since it needs to stay running
throughout the following steps.

In the `tools/websocket-logger/` directory:

```
npm ci
npm start
```

The output from this command will indicate where the results are being logged,
which will be needed later. For example:

```
...
Writing to wslog-2023-09-12T18-57-34.txt
...
```

See also [tools/websocket-logger/README.md](../tools/websocket-logger/README.md).

### Running CTS

Now we need to run the specific cases in CTS that we need to time.

This should be possible under any development workflow by logging through a
side-channel (as long as its runtime environment, like Node, supports WebSockets).
Regardless of development workflow, you need to enable logToWebSocket flag
(`?log_to_web_socket=1` in browser, `--log-to-web-socket` on command line, or
just hack it in by switching the default in `options.ts`).

The most well-tested way to do this is using the standalone web runner.

This requires serving the CTS locally. In the project root:

```
npm run standalone
npm start
```

Once this is started you can then direct a WebGPU enabled browser to the
specific CTS entry and run the tests, for example:

```
http://localhost:8080/standalone/?log_to_web_socket=1&q=webgpu:*
```

If the tests have a high variance in runtime, you can run them multiple times.
The longest recorded time will be used.

### Merging metadata

The final step is to merge the new data that has been captured into the JSON
file.

This can be done using the following command:

```
tools/merge_listing_times webgpu -- tools/websocket-logger/wslog-2023-09-12T18-57-34.txt
tools/merge_listing_times webgpu -- tools/websocket-logger/wslog-*.txt
```

Or, you can point it to one of the log files from a specific invocation of websocket-logger.

Now you just need to commit the pending diff in your repo.
