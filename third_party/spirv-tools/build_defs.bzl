COMMON_COPTS = [
        "-DSPIRV_CHECK_CONTEXT",
        "-DSPIRV_COLOR_TERMINAL",
    ] + select({
    "@bazel_tools//src/conditions:windows": [""],
    "//conditions:default": [
        "-DSPIRV_LINUX",
        "-DSPIRV_TIMER_ENABLED",
        "-Wall",
        "-Wextra",
        "-Wnon-virtual-dtor",
        "-Wno-missing-field-initializers",
        "-Werror",
        "-std=c++11",
        "-fvisibility=hidden",
        "-fno-exceptions",
        "-fno-rtti",
        "-Wno-long-long",
        "-Wshadow",
        "-Wundef",
        "-Wconversion",
        "-Wno-sign-conversion",
    ],
})

TEST_COPTS = COMMON_COPTS + select({
    "@bazel_tools//src/conditions:windows": [
        # Disable C4503 "decorated name length exceeded" warning,
        # triggered by some heavily templated types.
        # We don't care much about that in test code.
        # Important to do since we have warnings-as-errors.
        "/wd4503"
    ],
    "//conditions:default": [
        "-Wno-undef",
        "-Wno-self-assign",
        "-Wno-shadow",
        "-Wno-unused-parameter"
    ],
})

DEBUGINFO_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_debuginfo_grammar_unified1"
CLDEBUGINFO100_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_opencl_debuginfo_100_grammar_unified1"
SHDEBUGINFO100_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_nonsemantic_shader_debuginfo_100_grammar_unified1"

def generate_core_tables(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_core_grammar_" + version,
        DEBUGINFO_GRAMMAR_JSON_FILE,
        CLDEBUGINFO100_GRAMMAR_JSON_FILE,
    ]
    outs = [
        "core.insts-{}.inc".format(version),
        "operand.kinds-{}.inc".format(version),
    ]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_core_tables_" + version,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--spirv-core-grammar=$(location {0}) " +
            "--extinst-debuginfo-grammar=$(location {1}) " +
            "--extinst-cldebuginfo100-grammar=$(location {2}) " +
            "--core-insts-output=$(location {3}) " +
            "--operand-kinds-output=$(location {4})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_enum_string_mapping(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_core_grammar_" + version,
        DEBUGINFO_GRAMMAR_JSON_FILE,
        CLDEBUGINFO100_GRAMMAR_JSON_FILE,
    ]
    outs = [
        "extension_enum.inc",
        "enum_string_mapping.inc",
    ]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_enum_string_mapping",
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--spirv-core-grammar=$(location {0}) " +
            "--extinst-debuginfo-grammar=$(location {1}) " +
            "--extinst-cldebuginfo100-grammar=$(location {2}) " +
            "--extension-enum-output=$(location {3}) " +
            "--enum-string-mapping-output=$(location {4})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_opencl_tables(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_opencl_grammar_" + version,
    ]
    outs = ["opencl.std.insts.inc"]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_opencl_tables_" + version,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--extinst-opencl-grammar=$(location {0}) " +
            "--opencl-insts-output=$(location {1})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_glsl_tables(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_glsl_grammar_" + version,
    ]
    outs = ["glsl.std.450.insts.inc"]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_glsl_tables_" + version,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--extinst-glsl-grammar=$(location {0}) " +
            "--glsl-insts-output=$(location {1})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_vendor_tables(extension, operand_kind_prefix = ""):
    if not extension:
        fail("Must specify extension", "extension")
    extension_rule = extension.replace("-", "_").replace(".", "_")
    grammars = ["@spirv_headers//:spirv_ext_inst_{}_grammar_unified1".format(extension_rule)]
    outs = ["{}.insts.inc".format(extension)]
    prefices = [operand_kind_prefix]
    fmtargs = grammars + outs + prefices
    native.genrule(
        name = "gen_vendor_tables_" + extension_rule,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--extinst-vendor-grammar=$(location {0}) " +
            "--vendor-insts-output=$(location {1}) " +
            "--vendor-operand-kind-prefix={2}"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_extinst_lang_headers(name, grammar = None):
    if not grammar:
        fail("Must specify grammar", "grammar")
    outs = [name + ".h"]
    fmtargs = outs
    native.genrule(
        name = "gen_extinst_lang_headers_" + name,
        srcs = [grammar],
        outs = outs,
        cmd = (
            "$(location :generate_language_headers) " +
            "--extinst-grammar=$< " +
            "--extinst-output-path=$(location {0})"
        ).format(*fmtargs),
        tools = [":generate_language_headers"],
        visibility = ["//visibility:private"],
    )

def base_test(name, srcs, deps = []):
    if srcs == []:
        return
    if name[-5:] != "_test":
        name = name + "_test"
    native.cc_test(
        name = "base_" + name,
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "large",
        deps = [
            ":test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def lint_test(name, srcs, deps = []):
    if name[-5:] != "_test":
        name = name + "_test"
    native.cc_test(
        name = "lint_" + name,
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "large",
        deps = [
            ":spirv_tools_lint",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def link_test(name, srcs, deps = []):
    if name[-5:] != "_test":
        name = name + "_test"
    native.cc_test(
        name = "link_" + name,
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "large",
        deps = [
            ":link_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def opt_test(name, srcs, deps = []):
    if name[-5:] != "_test":
        name = name + "_test"
    native.cc_test(
        name = "opt_" + name,
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "large",
        deps = [
            ":opt_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def reduce_test(name, srcs, deps = []):
    if name[-5:] != "_test":
        name = name + "_test"
    native.cc_test(
        name = "reduce_" + name,
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "large",
        deps = [
            ":reduce_test_common",
            ":spirv_tools_reduce",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def util_test(name, srcs, deps = []):
    if name[-5:] != "_test":
        name = name + "_test"
    native.cc_test(
        name = "util_" + name,
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "large",
        deps = [
            ":opt_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def val_test(name, srcs = [], copts = [], deps = [], **kwargs):
    if name[-5:] != "_test":
        name = name + "_test"
    if name[:4] != "val_":
        name = "val_" + name
    native.cc_test(
        name = name,
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS + copts,
        size = "large",
        deps = [
            ":val_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
        **kwargs
    )
