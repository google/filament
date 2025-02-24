// RUN: %dxc -O0 -T lib_6_x %s | FileCheck %s
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd

cbuffer cb : register(b0) {
  float bar;
};

float foo() {
    float x = bar;

#if defined(__SHADER_TARGET_STAGE) && __SHADER_TARGET_STAGE == __SHADER_STAGE_LIBRARY
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MAJOR) && __SHADER_TARGET_MAJOR == 6
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MINOR) && __SHADER_TARGET_MINOR == 15
    x += 1;
#else
    x -= 1;
#endif

    return x;
}
