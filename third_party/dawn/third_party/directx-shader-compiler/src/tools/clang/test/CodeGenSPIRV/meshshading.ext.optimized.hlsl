// RUN: %dxc -T ms_6_6 -E main -spirv -fspv-target-env=vulkan1.3 %s | FileCheck %s

// CHECK:      OpExtension "SPV_EXT_mesh_shader"

struct Object
{
    float4 v: SV_POSITION;
};

RWStructuredBuffer<int> buffer;

// If optimizations are disabled, this function is not inlined.
// This means a RWStructuredBuffer will be created as a local variable,
// which is impossible in logical SPIR-V.
// Hence why should make sure the optimizations are enabled for mesh shaders.
void foo(RWStructuredBuffer<int> param) { }

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out indices uint3 idx[3], out vertices Object vtx[3])
{
  foo(buffer);
}
