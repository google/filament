load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

platform(
    name = "windows",
    constraint_values = [
        "@platforms//os:windows",
    ],
)

TEST_COPTS = [
    "-pedantic",
    "-pedantic-errors",
    "-std=c++11",
    "-Wall",
    "-Wconversion",
    "-Wextra",
    "-Wshadow",
    #    "-Wshorten-64-to-32",
    "-Wfloat-equal",
    "-fstrict-aliasing",
    ## assert() are used a lot in tests upstream, which may be optimised out leading to
    ## unused-variable warning.
    "-Wno-unused-variable",
    "-Werror=old-style-cast",
]

# Some of the issues with DoNotOptimize only occur when optimization is enabled
PER_SRC_COPTS = {
    "donotoptimize_test.cc": ["-O3"],
}

TEST_ARGS = ["--benchmark_min_time=0.01s"]

PER_SRC_TEST_ARGS = {
    "user_counters_tabular_test.cc": ["--benchmark_counters_tabular=true"],
    "repetitions_test.cc": [" --benchmark_repetitions=3"],
    "spec_arg_test.cc": ["--benchmark_filter=BM_NotChosen"],
    "spec_arg_verbosity_test.cc": ["--v=42"],
    "complexity_test.cc": ["--benchmark_min_time=1000000x"],
}

cc_library(
    name = "output_test_helper",
    testonly = 1,
    srcs = ["output_test_helper.cc"],
    hdrs = ["output_test.h"],
    copts = select({
        "//:windows": [],
        "//conditions:default": TEST_COPTS,
    }),
    deps = [
        "//:benchmark",
        "//:benchmark_internal_headers",
    ],
)

# Tests that use gtest.  These rely on `gtest_main`.
[
    cc_test(
        name = test_src[:-len(".cc")],
        size = "small",
        srcs = [test_src],
        copts = select({
            "//:windows": [],
            "//conditions:default": TEST_COPTS,
        }) + PER_SRC_COPTS.get(test_src, []),
        deps = [
            "//:benchmark",
            "//:benchmark_internal_headers",
            "@com_google_googletest//:gtest",
            "@com_google_googletest//:gtest_main",
        ],
    )
    for test_src in glob(["*_gtest.cc"])
]

# Tests that do not use gtest.  These have their own `main` defined.
[
    cc_test(
        name = test_src[:-len(".cc")],
        size = "small",
        srcs = [test_src],
        args = TEST_ARGS + PER_SRC_TEST_ARGS.get(test_src, []),
        copts = select({
            "//:windows": [],
            "//conditions:default": TEST_COPTS,
        }) + PER_SRC_COPTS.get(test_src, []),
        deps = [
            ":output_test_helper",
            "//:benchmark",
            "//:benchmark_internal_headers",
        ],
        # FIXME: Add support for assembly tests to bazel.
        # See Issue #556
        # https://github.com/google/benchmark/issues/556
    )
    for test_src in glob(
        ["*_test.cc"],
        exclude = [
            "*_assembly_test.cc",
            "cxx03_test.cc",
            "link_main_test.cc",
        ],
    )
]

cc_test(
    name = "cxx03_test",
    size = "small",
    srcs = ["cxx03_test.cc"],
    copts = TEST_COPTS + ["-std=c++03"],
    target_compatible_with = select({
        "//:windows": ["@platforms//:incompatible"],
        "//conditions:default": [],
    }),
    deps = [
        ":output_test_helper",
        "//:benchmark",
        "//:benchmark_internal_headers",
    ],
)

cc_test(
    name = "link_main_test",
    size = "small",
    srcs = ["link_main_test.cc"],
    copts = select({
        "//:windows": [],
        "//conditions:default": TEST_COPTS,
    }),
    deps = ["//:benchmark_main"],
)
