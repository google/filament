load("@nanobind_bazel//:build_defs.bzl", "nanobind_extension", "nanobind_stubgen")
load("@rules_python//python:defs.bzl", "py_library", "py_test")

py_library(
    name = "google_benchmark",
    srcs = ["__init__.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":_benchmark",
    ],
)

nanobind_extension(
    name = "_benchmark",
    srcs = ["benchmark.cc"],
    deps = ["//:benchmark"],
)

nanobind_stubgen(
    name = "benchmark_stubgen",
    marker_file = "bindings/python/google_benchmark/py.typed",
    module = ":_benchmark",
)

py_test(
    name = "example",
    srcs = ["example.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    visibility = ["//visibility:public"],
    deps = [
        ":google_benchmark",
    ],
)
