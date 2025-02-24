// RUN: %dxc -O0 -T ds_6_3 %s | FileCheck %s
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd

struct VSSceneIn {
  float3 pos : POSITION;
};

struct PSSceneIn {
  float4 pos : SV_Position;

uint   RTIndex      : SV_RenderTargetArrayIndex;
};

struct HSPerVertexData {
  PSSceneIn v;
};

struct HSPerPatchData {
  float edges[3] : SV_TessFactor;
  float inside : SV_InsideTessFactor;
};

// domain shader that actually outputs the triangle vertices
[domain("tri")] PSSceneIn main(const float3 bary
                               : SV_DomainLocation,
                                 const OutputPatch<HSPerVertexData, 3> patch,
                                 const HSPerPatchData perPatchData) {
  PSSceneIn v;
  float x = bary.x;

#if defined(__SHADER_TARGET_STAGE) && __SHADER_TARGET_STAGE == __SHADER_STAGE_DOMAIN
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MAJOR) && __SHADER_TARGET_MAJOR == 6
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MINOR) && __SHADER_TARGET_MINOR == 3
    x += 1;
#else
    x -= 1;
#endif

    v = patch[0].v;
    v.pos.x = x;

    return v;
}
