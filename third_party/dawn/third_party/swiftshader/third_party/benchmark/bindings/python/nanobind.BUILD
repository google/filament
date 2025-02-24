cc_library(
    name = "nanobind",
    srcs = glob([
        "src/*.cpp"
    ]),
    copts = ["-fexceptions"],
    includes = ["include", "ext/robin_map/include"],
    textual_hdrs = glob(
        [
            "include/**/*.h",
            "src/*.h",
            "ext/robin_map/include/tsl/*.h",
        ],
    ),
    deps = ["@python_headers"],
    visibility = ["//visibility:public"],
)
