"""Constants and macros for spirv-tools BUILD."""

COMMON_COPTS = [
    "-DSPIRV_CHECK_CONTEXT",
    "-DSPIRV_COLOR_TERMINAL",
] + select({
    "@platforms//os:windows": [],
    "//conditions:default": [
        "-DSPIRV_LINUX",
        "-DSPIRV_TIMER_ENABLED",
        "-fvisibility=hidden",
        "-fno-exceptions",
        "-fno-rtti",
        "-Wall",
        "-Wextra",
        "-Wnon-virtual-dtor",
        "-Wno-missing-field-initializers",
        "-Werror",
        "-Wno-long-long",
        "-Wshadow",
        "-Wundef",
        "-Wconversion",
        "-Wno-sign-conversion",
    ],
})

TEST_COPTS = COMMON_COPTS + [
] + select({
    "@platforms//os:windows": [
        # Disable C4503 "decorated name length exceeded" warning,
        # triggered by some heavily templated types.
        # We don't care much about that in test code.
        # Important to do since we have warnings-as-errors.
        "/wd4503",
    ],
    "//conditions:default": [
        "-Wno-undef",
        "-Wno-self-assign",
        "-Wno-shadow",
        "-Wno-unused-parameter",
    ],
})

def incompatible_with(incompatible_constraints):
    return select(_merge_dicts([{"//conditions:default": []}, {
        constraint: ["@platforms//:incompatible"]
        for constraint in incompatible_constraints
    }]))

DEBUGINFO_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_debuginfo_grammar_unified1"
CLDEBUGINFO100_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_opencl_debuginfo_100_grammar_unified1"
SHDEBUGINFO100_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_nonsemantic_shader_debuginfo_100_grammar_unified1"

def _merge_dicts(dicts):
    merged = {}
    for d in dicts:
        merged.update(d)
    return merged

def generate_core_tables(version):
    if not version:
        fail("Must specify version", "version")

    grammars = dict(
        core_grammar = "@spirv_headers//:spirv_core_grammar_{}".format(version),
        debuginfo_grammar = DEBUGINFO_GRAMMAR_JSON_FILE,
        cldebuginfo_grammar = CLDEBUGINFO100_GRAMMAR_JSON_FILE,
    )

    outs = dict(
        core_insts_output = "core.insts-{}.inc".format(version),
        operand_kinds_output = "operand.kinds-{}.inc".format(version),
    )

    cmd = (
        "$(location :generate_grammar_tables)" +
        " --spirv-core-grammar=$(location {core_grammar})" +
        " --extinst-debuginfo-grammar=$(location {debuginfo_grammar})" +
        " --extinst-cldebuginfo100-grammar=$(location {cldebuginfo_grammar})" +
        " --core-insts-output=$(location {core_insts_output})" +
        " --operand-kinds-output=$(location {operand_kinds_output})" +
        " --output-language=c++"
    ).format(**_merge_dicts([grammars, outs]))

    native.genrule(
        name = "gen_core_tables_" + version,
        srcs = grammars.values(),
        outs = outs.values(),
        cmd = cmd,
        cmd_bat = cmd,
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_enum_string_mapping(version):
    if not version:
        fail("Must specify version", "version")

    grammars = dict(
        core_grammar = "@spirv_headers//:spirv_core_grammar_{}".format(version),
        debuginfo_grammar = DEBUGINFO_GRAMMAR_JSON_FILE,
        cldebuginfo_grammar = CLDEBUGINFO100_GRAMMAR_JSON_FILE,
    )

    outs = dict(
        extension_enum_ouput = "extension_enum.inc",
        enum_string_mapping_output = "enum_string_mapping.inc",
    )

    cmd = (
        "$(location :generate_grammar_tables)" +
        " --spirv-core-grammar=$(location {core_grammar})" +
        " --extinst-debuginfo-grammar=$(location {debuginfo_grammar})" +
        " --extinst-cldebuginfo100-grammar=$(location {cldebuginfo_grammar})" +
        " --extension-enum-output=$(location {extension_enum_ouput})" +
        " --enum-string-mapping-output=$(location {enum_string_mapping_output})" +
        " --output-language=c++"
    ).format(**_merge_dicts([grammars, outs]))

    native.genrule(
        name = "gen_enum_string_mapping",
        srcs = grammars.values(),
        outs = outs.values(),
        cmd = cmd,
        cmd_bat = cmd,
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_opencl_tables(version):
    if not version:
        fail("Must specify version", "version")

    grammars = dict(
        opencl_grammar = "@spirv_headers//:spirv_opencl_grammar_{}".format(version),
    )

    outs = dict(
        opencl_insts_output = "opencl.std.insts.inc",
    )

    cmd = (
        "$(location :generate_grammar_tables)" +
        " --extinst-opencl-grammar=$(location {opencl_grammar})" +
        " --opencl-insts-output=$(location {opencl_insts_output})"
    ).format(**_merge_dicts([grammars, outs]))

    native.genrule(
        name = "gen_opencl_tables_" + version,
        srcs = grammars.values(),
        outs = outs.values(),
        cmd = cmd,
        cmd_bat = cmd,
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_glsl_tables(version):
    if not version:
        fail("Must specify version", "version")

    grammars = dict(
        gsls_grammar = "@spirv_headers//:spirv_glsl_grammar_{}".format(version),
    )
    outs = dict(
        gsls_insts_outs = "glsl.std.450.insts.inc",
    )

    cmd = (
        "$(location :generate_grammar_tables)" +
        " --extinst-glsl-grammar=$(location {gsls_grammar})" +
        " --glsl-insts-output=$(location {gsls_insts_outs})" +
        " --output-language=c++"
    ).format(**_merge_dicts([grammars, outs]))

    native.genrule(
        name = "gen_glsl_tables_" + version,
        srcs = grammars.values(),
        outs = outs.values(),
        cmd = cmd,
        cmd_bat = cmd,
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_vendor_tables(extension, operand_kind_prefix = ""):
    if not extension:
        fail("Must specify extension", "extension")

    extension_rule = extension.replace("-", "_").replace(".", "_")
    grammars = dict(
        vendor_grammar = "@spirv_headers//:spirv_ext_inst_{}_grammar_unified1".format(extension_rule),
    )
    outs = dict(
        vendor_insts_output = "{}.insts.inc".format(extension),
    )
    cmd = (
        "$(location :generate_grammar_tables)" +
        " --extinst-vendor-grammar=$(location {vendor_grammar})" +
        " --vendor-insts-output=$(location {vendor_insts_output})" +
        " --vendor-operand-kind-prefix={operand_kind_prefix}"
    ).format(operand_kind_prefix = operand_kind_prefix, **_merge_dicts([grammars, outs]))

    native.genrule(
        name = "gen_vendor_tables_" + extension_rule,
        srcs = grammars.values(),
        outs = outs.values(),
        cmd = cmd,
        cmd_bat = cmd,
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_extinst_lang_headers(name, grammar = None):
    if not grammar:
        fail("Must specify grammar", "grammar")
    outs = dict(
        extinst_output_path = name + ".h",
    )
    cmd = (
        "$(location :generate_language_headers)" +
        " --extinst-grammar=$<" +
        " --extinst-output-path=$(location {extinst_output_path})"
    ).format(**outs)

    native.genrule(
        name = "gen_extinst_lang_headers_{}".format(name),
        srcs = [grammar],
        outs = outs.values(),
        cmd = cmd,
        cmd_bat = cmd,
        tools = [":generate_language_headers"],
        visibility = ["//visibility:private"],
    )
