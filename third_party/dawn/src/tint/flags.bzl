load("@bazel_skylib//rules:common_settings.bzl", "string_flag", "bool_flag")

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

COPTS = [
    "-fno-rtti",
    "-fno-exceptions",
    "--std=c++20",
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
}) + select({
    "//src/tint:tint_build_null_writer_true": [ "-DTINT_BUILD_NULL_WRITER" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_fuzzer_vulkan_support_true": [ "-DTINT_BUILD_FUZZER_VULKAN_SUPPORT" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_ir_binary_true": [ "-DTINT_BUILD_IR_BINARY" ],
    "//conditions:default": [],
}) + select({
    "//src/tint:tint_build_mesa_true": [ "-DTINT_BUILD_MESA" ],
    "//conditions:default": [],
})
