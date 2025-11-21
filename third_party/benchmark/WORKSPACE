workspace(name = "com_github_google_benchmark")

load("//:bazel/benchmark_deps.bzl", "benchmark_deps")

benchmark_deps()

load("@rules_python//python:repositories.bzl", "py_repositories")

py_repositories()

load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    name = "tools_pip_deps",
    requirements_lock = "//tools:requirements.txt",
)

load("@tools_pip_deps//:requirements.bzl", "install_deps")

install_deps()
