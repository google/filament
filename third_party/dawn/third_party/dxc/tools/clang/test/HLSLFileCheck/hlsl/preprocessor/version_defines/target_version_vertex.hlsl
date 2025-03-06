// RUN: %dxc -O0 -T vs_6_0 %s | FileCheck %s
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd

float4 main(float4 pos : IN) : OUT
{
  float x = pos.x;
  float4 outPos = pos;

  // Will need to change if there's ever a major version bump
#if defined(__DXC_VERSION_MAJOR) && __DXC_VERSION_MAJOR == 1
  x += 1;
#else
  x -= 1;
#endif

  // Minor version is expected to change fairly frequently. So be conservative
#if defined(__DXC_VERSION_MINOR) && __DXC_VERSION_MINOR >= 0 && __DXC_VERSION_MINOR < 50
  x += 1;
#else
  x -= 1;
#endif

  // Release is either based on the year/month or set to zero for dev builds
#if defined(__DXC_VERSION_RELEASE) && (__DXC_VERSION_RELEASE == 0 ||  __DXC_VERSION_RELEASE > 1900)
  x += 1;
#else
  x -= 1;
#endif

  // The last number varies a lot. In a dev build, Release is 0, and this number
  // is the total number of commits since the beginning of the project.
  // which can be expected to be more than 1000 at least.
#if defined(__DXC_VERSION_COMMITS) && (__DXC_VERSION_RELEASE > 0 ||  __DXC_VERSION_COMMITS > 1000)
  x += 1;
#else
  x -= 1;
#endif
  // Default HLSL version should be the highest available version
#if defined(__HLSL_VERSION) && __HLSL_VERSION >= 2018
  x += 1;
#else
  x -= 1;
#endif
#if defined(__SHADER_TARGET_STAGE) && __SHADER_TARGET_STAGE == __SHADER_STAGE_VERTEX
  x += 1;
#else
  x -= 1;
#endif
// Compiler upgrades to 6.0 if less
#if defined(__SHADER_TARGET_MAJOR) && __SHADER_TARGET_MAJOR == 6
  x += 1;
#else
  x -= 1;
#endif
// Compiler upgrades to 6.0 if less
#if defined(__SHADER_TARGET_MINOR) && __SHADER_TARGET_MINOR == 0
  x += 1;
#else
  x -= 1;
#endif
  outPos.x = x;
  return outPos;
}
