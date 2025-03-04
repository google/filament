// RUN: %dxc -O0 -T ms_6_5 %s | FileCheck %s
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd

cbuffer cb : register(b0) {
  float foo;
};

struct Vertex { float4 pos : SV_Position; };

[outputtopology("point")]
[numthreads(2,2,1)]
void main(
    out vertices Vertex verts[1],
    out indices uint3 idx[1])
{
    verts = (Vertex[1])0;

    float x = foo;
#if defined(__SHADER_TARGET_STAGE) && __SHADER_TARGET_STAGE == __SHADER_STAGE_MESH
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MAJOR) && __SHADER_TARGET_MAJOR == 6
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MINOR) && __SHADER_TARGET_MINOR == 5
    x += 1;
#else
    x -= 1;
#endif
    verts[0].pos = x;
    SetMeshOutputCounts(0, 0);
}
