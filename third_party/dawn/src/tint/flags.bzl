load("@bazel_skylib//rules:common_settings.bzl", "string_flag", "bool_flag")
load("@bazel_skylib//lib:selects.bzl", "selects")

def declare_bool_flag(name, default):
    """Create a boolean flag and two config_settings with the names: <name>_true, <name>_false.

    declare_bool_flag is a Bazel Macro that defines a boolean flag with the given name two
    config_settings, one for True, one for False. Reminder that Bazel has special syntax for
    unsetting boolean flags, but this does not work well with aliases.
    https://docs.bazel.build/versions/main/skylark/config.html#using-build-settings-on-the-command-line
    Thus it is best to define both an "enabled" alias and a "disabled" alias.

    Args:
        name: string, the name of the flag to create and use for the config_settings
        default: boolean, if the flag should default to on or off.
    """

    bool_flag(name = name, build_setting_default = default)

    native.config_setting(
        name = name + "_true",
        flag_values = {
            ":" + name: "True",
        },
        visibility = ["//visibility:public"],
    )

    native.config_setting(
        name = name + "_false",
        flag_values = {
            ":" + name: "False",
        },
        visibility = ["//visibility:public"],
    )

def declare_os_flag():
    """Creates the 'os' string flag that specifies the OS to target, and a pair of
    'tint_build_is_<os>_true' and 'tint_build_is_<os>_false' targets.

    The OS flag can be specified on the command line with '--//src/tint:os=<os>'
    """

    OSes = [
        "win",
        "linux",
        "mac",
        "other"
    ]

    string_flag(
        name = "os",
        build_setting_default = "other",
        values = OSes,
    )

    for os in OSes:
        native.config_setting(
            name = "tint_build_is_{}_true".format(os),
            flag_values = { ":os": os },
            visibility = ["//visibility:public"],
        )
        selects.config_setting_group(
            name = "tint_build_is_{}_false".format(os),
            match_any = [ "tint_build_is_{}_true".format(other) for other in OSes if other != os],
            visibility = ["//visibility:public"],
        )

COPTS = [
    "-fno-rtti",
    "-fno-exceptions",
    "--std=c++17",
] + select({
    "//src/tint:tint_build_glsl_writer_true": [ "-DTINT_BUILD_GLSL_WRITER" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_hlsl_writer_true": [ "-DTINT_BUILD_HLSL_WRITER" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_ir_true":          [ "-DTINT_BUILD_IR" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_msl_writer_true":  [ "-DTINT_BUILD_MSL_WRITER" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_spv_reader_true":  [ "-DTINT_BUILD_SPV_READER" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_spv_writer_true":  [ "-DTINT_BUILD_SPV_WRITER" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_wgsl_reader_true": [ "-DTINT_BUILD_WGSL_READER" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_wgsl_writer_true": [ "-DTINT_BUILD_WGSL_WRITER" ],
    "//conditions:default": [],
})
