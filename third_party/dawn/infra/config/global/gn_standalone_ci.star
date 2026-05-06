# Copyright 2025 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""CI Dawn builders using GN and a standalone Dawn checkout (instead of Chromium)."""

load("@chromium-luci//args.star", "args")
load("@chromium-luci//builder_config.star", "builder_config")
load("@chromium-luci//builders.star", "os")
load("@chromium-luci//ci.star", "ci")
load("@chromium-luci//consoles.star", "consoles")
load("@chromium-luci//gardener_rotations.star", "gardener_rotations")
load("@chromium-luci//gn_args.star", "gn_args")
load("@chromium-luci//targets.star", "targets")
load("//constants.star", "siso")

ci.defaults.set(
    executable = "recipe:dawn/gn_v2",
    builder_group = "ci",
    bucket = "ci",
    pool = "luci.chromium.gpu.ci",
    triggered_by = ["primary-poller"],
    build_numbers = True,
    contact_team_email = "chrome-gpu-infra@google.com",
    service_account = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
    shadow_service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    siso_project = siso.project.DEFAULT_TRUSTED,
    shadow_siso_project = siso.project.DEFAULT_UNTRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
    thin_tester_cores = 2,
    builderless = True,
    notifies = ["gardener-notifier"],
    gardener_rotations = gardener_rotations.rotation("dawn", None, None),
)

targets.builder_defaults.set(
    mixins = [
        "chromium-tester-service-account",
        "swarming_containment_auto",
    ],
)

################################################################################
# Parent Builders                                                              #
################################################################################

def dawn_linux_parent_builder(**kwargs):
    kwargs.setdefault("cores", 8)
    kwargs.setdefault("os", os.LINUX_DEFAULT)
    ci.builder(**kwargs)

def dawn_mac_parent_builder(**kwargs):
    kwargs.setdefault("cpu", "arm64")
    kwargs.setdefault("os", os.MAC_DEFAULT)
    ci.builder(**kwargs)

def dawn_win_parent_builder(**kwargs):
    kwargs.setdefault("cores", 8)
    kwargs.setdefault("os", os.WINDOWS_DEFAULT)
    kwargs.setdefault("ssd", None)
    ci.builder(**kwargs)

dawn_linux_parent_builder(
    name = "dawn-linux-x64-builder-dbg",
    description_html = "Compile debug Dawn test binaries for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_node_bindings",
            "dawn_swiftshader",
            "linux_clang",
            "component",
            "debug",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|dbg",
        short_name = "x64",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x64-builder-rel",
    description_html = "Compiles release Dawn test binaries for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_node_bindings",
            "dawn_swiftshader",
            "linux_clang",
            "component",
            "release_with_dchecks",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel",
        short_name = "x64",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x64-builder-tsan",
    description_html = "Compiles release Dawn test binaries for Linux/x64 w/ TSAN enabled",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                # dawn_node is intentionally omitted since none of the tests
                # that are run with TSAN use node.
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_swiftshader",
            "linux_clang",
            "minimal_symbols",
            "non_component",
            "release_with_dchecks",
            "tsan",
            "x64",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|tsan",
        short_name = "x64",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x86-builder-dbg",
    description_html = "Compiles debug Dawn test binaries for Linux/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            # dawn_node is intentionally omitted since the original standalone
            # Linux/x86/Clang builders did not build/test with node.
            apply_configs = [
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_swiftshader",
            "debug",
            "linux_clang",
            "component",
            "x86",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|dbg",
        short_name = "x86",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x86-builder-rel",
    description_html = "Compiles release Dawn test binaries for Linux/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            # dawn_node is intentionally omitted since the original standalone
            # Linux/x86/Clang builders did not build/test with node.
            apply_configs = [
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "dawn_swiftshader",
            "linux_clang",
            "component",
            "release_with_dchecks",
            "x86",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel",
        short_name = "x86",
    ),
)

dawn_mac_parent_builder(
    name = "dawn-mac-arm64-builder-rel",
    description_html = "Compiles release Dawn test binaries for Mac/arm64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "arm64",
            "component",
            "dawn_node_bindings",
            "mac_clang",
            "release_with_dchecks",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|build|clang|rel",
        short_name = "a64",
    ),
)

dawn_mac_parent_builder(
    name = "dawn-mac-x64-builder-dbg",
    description_html = "Compiles debug Dawn test binaries for Mac/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_node_bindings",
            "debug",
            "mac_clang",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|build|clang|dbg",
        short_name = "x64",
    ),
)

dawn_mac_parent_builder(
    name = "dawn-mac-x64-builder-rel",
    description_html = "Compiles release Dawn test binaries for Mac/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
                "dawn_wasm",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_node_bindings",
            "mac_clang",
            "release_with_dchecks",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|build|clang|rel",
        short_name = "x64",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-arm64-builder-rel",
    description_html = "Compiles release Dawn test binaries for Windows/arm64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_node_bindings",
            "dawn_swiftshader",
            "release_with_dchecks",
            "win_clang",
            "arm64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|clang|rel",
        short_name = "a64",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-x64-builder-asan",
    description_html = "Compiles release Dawn test binaries for Windows/x64 with ASAN enabled",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "asan",
            "component",
            "dawn_node_bindings",
            "dawn_swiftshader",
            "release_with_dchecks",
            "win_clang",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|clang|asan",
        short_name = "x64",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-x64-builder-dbg",
    description_html = "Compiles debug Dawn test binaries for Windows/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_node_bindings",
            "dawn_swiftshader",
            "debug",
            "win_clang",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|clang|dbg",
        short_name = "x64",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-x64-builder-msvc-dbg",
    description_html = "Compiles debug Dawn test binaries for Windows/x64 using MSVC",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            # Component builds cause compile failures for the `default` target
            # with MSVC, so use non_component. See crbug.com/449779009.
            "dawn_node_bindings",
            "dawn_swiftshader",
            "debug",
            "non_component",
            "win_msvc",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|msvc|dbg",
        short_name = "x64",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-x64-builder-msvc-rel",
    description_html = "Compiles release Dawn test binaries for Windows/x64 using MSVC",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            # Component builds cause compile failures for the `default` target
            # with MSVC, so use non_component. See crbug.com/449779009.
            "dawn_node_bindings",
            "dawn_swiftshader",
            "non_component",
            "release_with_dchecks",
            "win_msvc",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|msvc|rel",
        short_name = "x64",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-x64-builder-rel",
    description_html = "Compiles release Dawn test binaries for Windows/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [
                "dawn_node",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_node_bindings",
            "dawn_swiftshader",
            "release_with_dchecks",
            "win_clang",
            "x64",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|clang|rel",
        short_name = "x64",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-x86-builder-dbg",
    description_html = "Compiles debug Dawn test binaries for Windows/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            # dawn_node is intentionally omitted since the original standalone
            # Win/x86/Clang builders did not build/test with node.
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_swiftshader",
            "debug",
            "win_clang",
            "x86",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|clang|dbg",
        short_name = "x86",
    ),
)

dawn_win_parent_builder(
    name = "dawn-win-x86-builder-rel",
    description_html = "Compiles release Dawn test binaries for Windows/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            # dawn_node is intentionally omitted since the original standalone
            # Win/x86/Clang builders did not build/test with node.
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_swiftshader",
            "release_with_dchecks",
            "win_clang",
            "x86",
        ],
    ),
    targets = targets.bundle(
        additional_compile_targets = [
            "default",
        ],
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|clang|rel",
        short_name = "x86",
    ),
)

################################################################################
# Fuzz Builders                                                                #
################################################################################

dawn_linux_parent_builder(
    name = "dawn-linux-x64-fuzz-dbg",
    description_html = "Compiles and runs debug Dawn binaries for 'tools/run fuzz' for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "libfuzzer",
            "linux_clang",
            "non_component",
            "debug",
            "x64",
        ],
    ),
    targets = targets.bundle(
        targets = [
            "tint_fuzzer_corpus_check_tests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|dbg|fuzz",
        short_name = "x64",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x64-fuzz-rel",
    description_html = "Compiles and runs release Dawn binaries for 'tools/run fuzz' for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "libfuzzer",
            "linux_clang",
            "non_component",
            "release_with_dchecks",
            "x64",
        ],
    ),
    targets = targets.bundle(
        targets = [
            "tint_fuzzer_corpus_check_tests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel|fuzz",
        short_name = "x64",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x86-fuzz-dbg",
    description_html = "Compiles and runs debug Dawn binaries for 'tools/run fuzz' for Linux/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "debug",
            "libfuzzer",
            "linux_clang",
            "non_component",
            "x86",
        ],
    ),
    targets = targets.bundle(
        targets = [
            "tint_fuzzer_corpus_check_tests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|dbg|fuzz",
        short_name = "x86",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x86-fuzz-rel",
    description_html = "Compiles and runs release Dawn binaries for 'tools/run fuzz' for Linux/x86",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "libfuzzer",
            "linux_clang",
            "non_component",
            "release_with_dchecks",
            "x86",
        ],
    ),
    targets = targets.bundle(
        targets = [
            "tint_fuzzer_corpus_check_tests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel|fuzz",
        short_name = "x86",
    ),
)

dawn_linux_parent_builder(
    name = "dawn-linux-x64-sws-clusterfuzz",
    description_html = "Generates ClusterFuzz corpora using Linux/x64 binaries and data from running with SwiftShader",
    # Run daily at 5PM Pacific.
    schedule = "0 0 * * *",
    triggered_by = [],
    gardener_rotations = args.ignore_default(None),
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "component",
            "dawn_swiftshader",
            "linux_clang",
            "release_with_dchecks",
            "tint_build_ir_binary",
            "x64",
        ],
    ),
    targets = targets.bundle(
        targets = [
            "tint_fuzzer_corpus_generate_tests",
            "wire_trace_gtests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel|cf",
        short_name = "x64",
    ),
)

################################################################################
# Child Testers                                                                #
################################################################################

ci.thin_tester(
    name = "dawn-linux-x64-intel-uhd630-rel",
    description_html = "Tests release Dawn on Linux/x64 on Intel CPUs w/ UHD 630 GPUs",
    parent = "dawn-linux-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "linux_intel_uhd_630_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x64",
        short_name = "630",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-intel-uhd770-rel",
    description_html = "Tests release Dawn on Linux/x64 on Intel CPUs w/ UHD 770 GPUs",
    parent = "dawn-linux-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "linux_intel_uhd_770_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x64",
        short_name = "770",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-nvidia-gtx1660-rel",
    description_html = "Tests release Dawn on Linux/x64 on NVIDIA GTX 1660 GPUs",
    parent = "dawn-linux-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "linux_nvidia_gtx_1660_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x64",
        short_name = "1660",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-sws-dbg",
    description_html = "Tests debug Dawn on Linux/x64 with SwiftShader",
    parent = "dawn-linux-x64-builder-dbg",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "swiftshader_gtests",
            "swiftshader_isolated_scripts",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|dbg|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-sws-rel",
    description_html = "Tests release Dawn on Linux/x64 with SwiftShader",
    parent = "dawn-linux-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "swiftshader_gtests",
            "swiftshader_isolated_scripts",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-sws-tsan",
    description_html = "Tests release Dawn on Linux/x64 with SwiftShader with TSAN",
    parent = "dawn-linux-x64-builder-tsan",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "dawn_end2end_sws_tsan_gtests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|tsan|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x86-sws-dbg",
    description_html = "Tests debug Dawn on Linux/x86 with SwiftShader",
    parent = "dawn-linux-x86-builder-dbg",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "swiftshader_gtests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|dbg|x86",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x86-sws-rel",
    description_html = "Tests release Dawn on Linux/x86 with SwiftShader",
    parent = "dawn-linux-x86-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "swiftshader_gtests",
        ],
        mixins = [
            "gpu_linux_gce_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.LINUX,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x86",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-mac-arm64-apple-m2-rel",
    description_html = "Tests release Dawn on Mac/arm64 on Apple M2 devices",
    parent = "dawn-mac-arm64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "mac_arm64_apple_m2_retina_gpu_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.MAC,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|test|clang|rel|arm64",
        short_name = "m2",
    ),
)

ci.thin_tester(
    name = "dawn-mac-x64-amd-5300m-rel",
    description_html = "Tests release Dawn on Mac/x64 on 16\" 2019 Macbook Pros w/ 5300M GPUs",
    parent = "dawn-mac-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "mac_retina_amd_gpu_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.MAC,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|test|clang|rel|x64",
        short_name = "5300m",
    ),
)

ci.thin_tester(
    name = "dawn-mac-x64-amd-555x-rel",
    description_html = "Tests release Dawn on Mac/x64 on 15\" 2019 Macbook Pros w/ AMD Radeon Pro 555X GPUs",
    parent = "dawn-mac-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "mac_retina_amd_555x_gpu_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.MAC,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|test|clang|rel|x64",
        short_name = "555x",
    ),
)

ci.thin_tester(
    name = "dawn-mac-x64-intel-uhd630-exp-rel",
    description_html = "Tests release Dawn on Mac/x64 on 2018 Mac Minis w/ Intel UHD 630 GPUs w/ experimental OS configs",
    parent = "dawn-mac-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "mac_mini_intel_gpu_experimental",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.MAC,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|test|clang|exp|x64",
        short_name = "630",
    ),
)

ci.thin_tester(
    name = "dawn-mac-x64-intel-uhd630-rel",
    description_html = "Tests release Dawn on Mac/x64 on 2018 Mac Minis w/ Intel UHD 630 GPUs",
    parent = "dawn-mac-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_common_gtests",
        ],
        mixins = [
            "mac_mini_intel_gpu_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.MAC,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|test|clang|rel|x64",
        short_name = "630",
    ),
)

ci.thin_tester(
    name = "dawn-mac-x64-sws-dbg",
    description_html = "Tests debug Dawn on Mac/x64 with SwiftShader",
    parent = "dawn-mac-x64-builder-dbg",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "swiftshader_gtests",
            "swiftshader_isolated_scripts",
        ],
        mixins = [
            "mac_mini_intel_gpu_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.MAC,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|test|clang|dbg|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-mac-x64-sws-rel",
    description_html = "Tests release Dawn on Mac/x64 with SwiftShader",
    parent = "dawn-mac-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "swiftshader_gtests",
            "swiftshader_isolated_scripts",
        ],
        mixins = [
            "mac_mini_intel_gpu_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.MAC,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|test|clang|rel|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-win-arm64-qualcomm-snapdragonxelite-rel",
    description_html = "Tests release Dawn on Windows/arm64 on devices with Snapdragon X Elite SoCs",
    parent = "dawn-win-arm64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win11_qualcomm_snapdragon_x_elite_stable",
            "win_snapdragon_x_elite_gtest_args",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|arm64",
        short_name = "sxe",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-amd-rx5500xt-rel",
    description_html = "Tests release Dawn on Windows/x64 on AMD RX 5500 XT GPUs",
    parent = "dawn-win-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win11_amd_rx_5500_xt_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x64",
        short_name = "5500",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-intel-uhd630-asan",
    description_html = "Tests release Dawn on Windows/x64/ASAN on Intel CPUs w/ UHD 630 GPUs",
    parent = "dawn-win-x64-builder-asan",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win10_intel_uhd_630_stable",
        ],
        per_test_modifications = {
            "dawn_end2end_no_dxc_validation_layers_tests": targets.remove(
                reason = "Removed from ASan testers for capacity reasons.",
            ),
        },
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|asan|x64",
        short_name = "630",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-intel-uhd630-rel",
    description_html = "Tests release Dawn on Windows/x64 on Intel CPUs w/ UHD 630 GPUs",
    parent = "dawn-win-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win10_intel_uhd_630_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x64",
        short_name = "630",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-intel-uhd770-rel",
    description_html = "Tests release Dawn on Windows/x64 on Intel CPUs w/ UHD 770 GPUs",
    parent = "dawn-win-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
        ],
        mixins = [
            "win10_intel_uhd_770_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x64",
        short_name = "770",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-nvidia-gtx1660-asan",
    description_html = "Tests release Dawn on Windows/x64/ASAN on NVIDIA GTX 1660 GPUs",
    parent = "dawn-win-x64-builder-asan",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win10_nvidia_gtx_1660_stable",
        ],
        per_test_modifications = {
            "dawn_end2end_no_dxc_validation_layers_tests": targets.remove(
                reason = "Removed from ASan testers for capacity reasons.",
            ),
        },
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|asan|x64",
        short_name = "1660",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-nvidia-gtx1660-exp-rel",
    description_html = "Tests release Dawn on Windows/x64 on NVIDIA GTX 1660 GPUs w/ experimental OS/driver configs",
    parent = "dawn-win-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win10_nvidia_gtx_1660_experimental",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|exp|x64",
        short_name = "1660",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-nvidia-gtx1660-rel",
    description_html = "Tests release Dawn on Windows/x64 on NVIDIA GTX 1660 GPUs",
    parent = "dawn-win-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win10_nvidia_gtx_1660_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x64",
        short_name = "1660",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-sws-dbg",
    description_html = "Tests debug Dawn on Windows/x64 with SwiftShader",
    parent = "dawn-win-x64-builder-dbg",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "win_software_renderer_gtests",
            "win_software_renderer_isolated_scripts",
        ],
        mixins = [
            "win10_gce_gpu_pool",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|dbg|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-sws-msvc-dbg",
    description_html = "Tests debug Dawn on Windows/x64 with SwiftShader using binaries built with MSVC",
    parent = "dawn-win-x64-builder-msvc-dbg",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "win_software_renderer_gtests",
            "win_software_renderer_isolated_scripts",
        ],
        mixins = [
            "win10_gce_gpu_pool",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|msvc|dbg|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-sws-msvc-rel",
    description_html = "Tests release Dawn on Windows/x64 with SwiftShader using binaries built with MSVC",
    parent = "dawn-win-x64-builder-msvc-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "win_software_renderer_gtests",
            "win_software_renderer_isolated_scripts",
        ],
        mixins = [
            "win10_gce_gpu_pool",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|msvc|rel|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-win-x64-sws-rel",
    description_html = "Tests release Dawn on Windows/x64 with SwiftShader",
    parent = "dawn-win-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "win_software_renderer_gtests",
            "win_software_renderer_isolated_scripts",
        ],
        mixins = [
            "win10_gce_gpu_pool",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x64",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-win-x86-intel-uhd630-rel",
    description_html = "Tests release Dawn on Windows/x86 on Intel CPUs w/ UHD 630 GPUs",
    parent = "dawn-win-x86-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win10_intel_uhd_630_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x86",
        short_name = "630",
    ),
)

ci.thin_tester(
    name = "dawn-win-x86-nvidia-gtx1660-rel",
    description_html = "Tests release Dawn on Windows/x86 on NVIDIA GTX 1660 GPUs",
    parent = "dawn-win-x86-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "real_hardware_win_gtests",
        ],
        mixins = [
            "win10_nvidia_gtx_1660_stable",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x86",
        short_name = "1660",
    ),
)

ci.thin_tester(
    name = "dawn-win-x86-sws-dbg",
    description_html = "Tests debug Dawn on Windows/x86 with SwiftShader",
    parent = "dawn-win-x86-builder-dbg",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "win_software_renderer_gtests",
        ],
        mixins = [
            "win10_gce_gpu_pool",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.DEBUG,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|dbg|x86",
        short_name = "sws",
    ),
)

ci.thin_tester(
    name = "dawn-win-x86-sws-rel",
    description_html = "Tests release Dawn on Windows/x86 with SwiftShader",
    parent = "dawn-win-x86-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
        run_tests_serially = True,
    ),
    targets = targets.bundle(
        targets = [
            "win_software_renderer_gtests",
        ],
        mixins = [
            "win10_gce_gpu_pool",
        ],
    ),
    targets_settings = targets.settings(
        browser_config = targets.browser_config.RELEASE,
        os_type = targets.os_type.WINDOWS,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|test|clang|rel|x86",
        short_name = "sws",
    ),
)
