load("//bindings/python:build_defs.bzl", "py_extension")

py_library(
    name = "google_benchmark",
    srcs = ["__init__.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":_benchmark",
    ],
)

py_extension(
    name = "_benchmark",
    srcs = ["benchmark.cc"],
    copts = [
        "-fexceptions",
        "-fno-strict-aliasing",
    ],
    features = [
        "-use_header_modules",
        "-parse_headers",
    ],
    deps = [
        "//:benchmark",
        "@nanobind",
        "@python_headers",
    ],
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

