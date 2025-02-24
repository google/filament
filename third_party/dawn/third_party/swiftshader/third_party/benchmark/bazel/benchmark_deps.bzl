load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def benchmark_deps():
    """Loads dependencies required to build Google Benchmark."""

    if "bazel_skylib" not in native.existing_rules():
        http_archive(
            name = "bazel_skylib",
            sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
            urls = [
                "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
                "https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
            ],
        )

    if "rules_foreign_cc" not in native.existing_rules():
        http_archive(
            name = "rules_foreign_cc",
            sha256 = "bcd0c5f46a49b85b384906daae41d277b3dc0ff27c7c752cc51e43048a58ec83",
            strip_prefix = "rules_foreign_cc-0.7.1",
            url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.7.1.tar.gz",
        )

    if "rules_python" not in native.existing_rules():
        http_archive(
            name = "rules_python",
            url = "https://github.com/bazelbuild/rules_python/releases/download/0.1.0/rules_python-0.1.0.tar.gz",
            sha256 = "b6d46438523a3ec0f3cead544190ee13223a52f6a6765a29eae7b7cc24cc83a0",
        )

    if "com_google_absl" not in native.existing_rules():
        http_archive(
            name = "com_google_absl",
            sha256 = "f41868f7a938605c92936230081175d1eae87f6ea2c248f41077c8f88316f111",
            strip_prefix = "abseil-cpp-20200225.2",
            urls = ["https://github.com/abseil/abseil-cpp/archive/20200225.2.tar.gz"],
        )

    if "com_google_googletest" not in native.existing_rules():
        new_git_repository(
            name = "com_google_googletest",
            remote = "https://github.com/google/googletest.git",
            tag = "release-1.11.0",
        )

    if "nanobind" not in native.existing_rules():
        new_git_repository(
            name = "nanobind",
            remote = "https://github.com/wjakob/nanobind.git",
            tag = "v1.4.0",
            build_file = "@//bindings/python:nanobind.BUILD",
            recursive_init_submodules = True,
        )

    if "libpfm" not in native.existing_rules():
        # Downloaded from v4.9.0 tag at https://sourceforge.net/p/perfmon2/libpfm4/ref/master/tags/
        http_archive(
            name = "libpfm",
            build_file = str(Label("//tools:libpfm.BUILD.bazel")),
            sha256 = "5da5f8872bde14b3634c9688d980f68bda28b510268723cc12973eedbab9fecc",
            type = "tar.gz",
            strip_prefix = "libpfm-4.11.0",
            urls = ["https://sourceforge.net/projects/perfmon2/files/libpfm4/libpfm-4.11.0.tar.gz/download"],
        )
