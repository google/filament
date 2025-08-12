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

SPIRV_CORE_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_core_grammar_unified1"
DEBUGINFO_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_debuginfo_grammar_unified1"
CLDEBUGINFO100_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_opencl_debuginfo_100_grammar_unified1"
SHDEBUGINFO100_GRAMMAR_JSON_FILE = "@spirv_headers//:spirv_ext_inst_nonsemantic_shader_debuginfo_100_grammar_unified1"

def _merge_dicts(dicts):
    merged = {}
    for d in dicts:
        merged.update(d)
    return merged


def ExtInst(name, target = "", prefix =""):
    """
    Returns a dictionary specifying the info needed to
    process an extended instruction set.

    Args:
        name: The extension name; forms part of the .json grammar file.
        target: if non-empty, the name of the bazel target in spirv-headers
           that names the JSON grammar file for the extended instrution set.
           If empty, the target name is derived from 'name'.
        prefix: The optional prefix for names of operand enums.

    Returns a dictionary with keys 'name', 'target', 'prefix' and the
    corresponding values.
    """
    return {'name':name, 'target':target, 'prefix':prefix}


def _extinst_grammar_target(e):
    """
    Args: e, as returned from extinst
    Returns the SPIRV-Headers target for the given extended instruction set spec.
    """
    target = e['target']
    name = e['name']
    if len(target) > 0:
        return "@spirv_headers//:{}".format(target)
    name_part = name.replace("-", "_").replace(".", "_")
    return "@spirv_headers//:spirv_ext_inst_{}_grammar_unified1".format(name_part)


def create_grammar_tables_target(name, extinsts):
    """
    Creates a ":gen_compressed_tables" target for SPIR-V instruction
    set grammar tables.

    Args:
        name: unused. Required by convention.
        extinsts: list of extended instruction specs.
            Each spec is a dictionary, as returned from 'extinst'.
    """
    grammars = dict(
        core_grammar = SPIRV_CORE_GRAMMAR_JSON_FILE,
    )
    outs = dict(
        core_tables_header_output = "core_tables_header.inc",
        core_tables_body_output = "core_tables_body.inc",
    )
    extinst_args = []
    for e in extinsts:
        extinst_args.append('--extinst={},$(location {})'.format(e['prefix'],_extinst_grammar_target(e)))

    cmd = (
        "$(location :ggt)" +
        " --spirv-core-grammar=$(location {core_grammar})" +
        " --core-tables-body-output=$(location {core_tables_body_output})" +
        " --core-tables-header-output=$(location {core_tables_header_output})" +
        " " + " ".join(extinst_args)
    ).format(**_merge_dicts([grammars, outs]))

    native.genrule(
        name = "gen_compressed_tables",
        srcs = grammars.values() + [_extinst_grammar_target(e) for e in extinsts],
        outs = outs.values(),
        cmd = cmd,
        cmd_bat = cmd,
        tools = [":ggt"],
        visibility = ["//visibility:private"],
    )

def generate_extinst_lang_headers(name, grammar = None):
    """
    Creates a :gen_extinst_lang_headers_* target for a C++ header
    the enums in a SPIR-V extended instruction set.

    Args:
       name: the basename of the emitted header file.
       grammar: the path to the JSON grammar file for the extended
           instruction set.
    """
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
