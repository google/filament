# dEQP Support

ANGLE integrates dEQP (i.e. the OpenGL CTS) for conformance testing.  It uses gtest to run tests,
and provides the means for dEQP to use ANGLE.

## Overriding dEQP files

Occasionally, ANGLE overrides certain dEQP files by copying them to this directory, adding the
`_override` suffix, and modifying them.  `deqp.gni` is used to select these override files to be
built with dEQP instead of the original files.

This is primarily done to fix tests until they are fixed upstream.

## Expectation files format

For every set of dEQP tests, for example GLES3 tests on the Vulkan backend, an expectations file
exists to let the test harness know which tests it should skip (as they are known to crash), or
expect to see failed.  Warnings are generated if a test unexpectedly passes, but an unexpected
failure is an error.  This let's ANGLE ensure there are no regressions.

If multiple test expectations in a file match a specific test due to
wildcards, the test harness picks the first match in the file as the
overriding expectation.

While developing a feature, or testing on a new platform, the expectations files can be modified to
reflect the reality of the situation.  The expected format for every line in these files is:

    {BUG#} {MODIFIERS} : {TEST_NAME} = {PASS,FAIL,FLAKY,TIMEOUT,SKIP}

`MODIFIERS` can be a combination of the below list, combined with a logical AND:

    WIN XP VISTA WIN7 WIN8 WIN10
    MAC LEOPARD SNOWLEOPARD LION MOUNTAINLION MAVERICKS YOSEMITE ELCAPITAN SIERRA HIGHSIERRA MOJAVE
    LINUX CHROMEOS ANDROID
    NVIDIA AMD INTEL
    DEBUG RELEASE
    D3D9 D3D11 OPENGL GLES VULKAN WGPU
    NEXUS5X PIXEL2ORXL PIXEL4ORXL PIXEL6
    QUADROP400
    SWIFTSHADER
    PREROTATION PREROTATION90 PREROTATION180 PREROTATION270
    NOSAN ASAN TSAN UBSAN

`TEST_NAME` can be a specific test name, or set of test names using `'*'` as wildcard anywhere in
the name.  Examples:

    // Disabled everywhere as is too slow:
    3445 : dEQP-GLES31.functional.ssbo.layout.random.all_shared_buffer.48 = SKIP

    // Crashes on both D3D11 and OPENGL:
    1442 OPENGL : dEQP-GLES31.functional.separate_shader.* = SKIP
    1442 D3D11 : dEQP-GLES31.functional.separate_shader.* = SKIP

    // Unsupported feature:
    3726 VULKAN ANDROID : dEQP-GLES31.functional.synchronization.inter_call.without_memory_barrier.*atomic_counter* = FAIL

    // Failing test in Nvidia's OpenGL implementation on windows:
    1665 WIN NVIDIA OPENGL : dEQP-GLES31.functional.draw_indirect.negative.command_offset_not_in_buffer_unsigned32_wrap = FAIL

    // Failing when emulated pre-rotation is enabled, no matter which angle:
    1234 PREROTATION : dEQP-GLES3.*blit* = FAIL

    // Failing when emulated pre-rotation is enabled with 270 degree angle:
    1234 PREROTATION270 : dEQP-GLES3.*blit* = FAIL

    // Flaky when run with thread-sanitizer (TSan)
    6678 TSAN : dEQP-EGL.functional.sharing.gles2.multithread.random* = FLAKY
