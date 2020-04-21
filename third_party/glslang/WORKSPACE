workspace(name = "org_khronos_glslang")
load(
    "@bazel_tools//tools/build_defs/repo:http.bzl",
    "http_archive",
)

http_archive(
    name = "com_google_googletest",
    sha256 = "ef9e2e12e7bf115ee48b427ae171fc869eeaf1b532c0fcfd982f6a353d2471b4",
    strip_prefix = "googletest-37ae1fc5e6be26f367d76c078beabd7024fed53a",
    urls = ["https://github.com/google/googletest/archive/37ae1fc5e6be26f367d76c078beabd7024fed53a.zip"],  # 2018-07-16
)

http_archive(
    name = "com_googlesource_code_re2",
    sha256 = "b885bb965ab4b6cf8718bbb8154d8f6474cd00331481b6d3e390babb3532263e",
    strip_prefix = "re2-e860767c86e577b87deadf24cc4567ea83c4f162/",
    urls = ["https://github.com/google/re2/archive/e860767c86e577b87deadf24cc4567ea83c4f162.zip"],
)

http_archive(
    name = "com_google_effcee",
    build_file = "BUILD.effcee.bazel",
    sha256 = "b0c21a01995fdf9792510566d78d5e7fe6f83cb4ba986eba691f4926f127cb34",
    strip_prefix = "effcee-8f0a61dc95e0df18c18e0ac56d83b3fa9d2fe90b/",
    urls = ["https://github.com/google/effcee/archive/8f0a61dc95e0df18c18e0ac56d83b3fa9d2fe90b.zip"],
)
