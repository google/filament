Quick descriptions of the scripts in tracing/bin/:

 * `chartjson2histograms`: Converts a chartjson file to HistogramSet JSON.
 * `compare_samples`: Compares metric results between two runs. Supports
   chart-json, HistogramSet JSON, and buildbot formats.
 * `generate_about_tracing_contents`: Vulcanizes trace viewer.
 * `histograms2csv`: Converts HistogramSet JSON to CSV.
 * `histograms2html`: Vulcanizes results.html. Optionally copies HistogramSet
   JSON from existing results.html and/or histograms.json file.
 * `html2trace`: Extracts trace JSON from a vulcanized trace HTML file.
 * `label_histograms`: Add a label to Histograms in an HTML or JSON file.
 * `map_traces`: Runs a trace map function over multiple traces. See also
   `run_metric`.
 * `memory_infra_remote_dump`: Extracts before/after memory dumps from a
   devtools remote protocol port.
 * `merge_histograms`: Merges Histograms from HistogramSet JSON according to a
   sequence of grouping keys, produces a new HistogramSet JSON.
 * `merge_traces`: Merge traces from either vulcanized HTML or JSON files to
   either a vulcanized HTML or a JSON file.
 * `results2json`: Extracts HistogramSet JSON from a results.html file.
 * `run_dev_server_tests`: Automatically run tracing dev server tests.
 * `run_metric`: Run a metric over one or more traces, produce vulcanized
   results.html.
 * `run_node_tests`: Automatically run headless tracing tests in node.
 * `run_py_tests`: Automatically run tracing python tests.
 * `run_tests`: Automatically run all tracing tests.
 * `run_vinn_tests`: Automatically run headless tracing tests in vinn.
 * `slim_trace`: Reads trace data from either HTML or JSON, removes some data,
   writes a new `slimmed_$filename` file.
 * `strip_memory_infra_trace`: Reads memory trace JSON, removes some data,
   writes a new `$filename-filtered.json` file.
 * `symbolize_trace`: Modifies trace JSON to symbolize symbols using a Chromium
   Debug build output directory.
 * `trace2html`: Vulcanizes trace data from a JSON file to an HTML file.
 * `update_gni`: Updates `trace_viewer.gni`.
 * `validate_all_diagnostics`: Checks that all Diagnostic classes in
   `tracing/tracing/value/diagnostics/` are registered correctly.
 * `validate_all_metrics`: Checks that all metric functions in
   `tracing/tracing/metrics/` are registered correctly.
 * `vulcanize_trace_viewer`: Vulcanizes trace viewer. (TODO(benjhayden): What is
   the difference between this and `generate_about_tracing_contents`?)
 * `why_imported`: Explain why given modules are imported in trace viewer.
