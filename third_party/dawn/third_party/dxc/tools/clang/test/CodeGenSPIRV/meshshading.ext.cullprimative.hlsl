// RUN: %dxc -T ms_6_6 -fspv-target-env=vulkan1.1spirv1.4 -E main %s -spirv | FileCheck %s

struct MeshletPrimitiveOut
{
	bool m_cullPrimitive : SV_CULLPRIMITIVE;
};

// m_cullPrimitive will have to be turned into a uint because of the Vulkan
// specification says that externally visible variables cannot be bool.
// CHECK: OpDecorate [[var:%[0-9]+]] BuiltIn CullPrimitiveEXT
// CHECK: OpDecorate [[var]] PerPrimitiveEXT
// CHECK: [[var]] = OpVariable %_ptr_Output__arr_bool_uint_2 Output

struct VertOut
{
    float4 m_svPosition : SV_POSITION;
};

#define SIZE 2

[numthreads(SIZE, 1, 1)] [outputtopology("triangle")]
void main(uint svGroupIndex : SV_GROUPINDEX, 
    out vertices VertOut verts[SIZE], 
    out indices uint3 indices[SIZE], 
    out primitives MeshletPrimitiveOut primitives[SIZE])
{
    SetMeshOutputCounts(SIZE, SIZE);

// Make sure that the references to m_cullPrimitive use uints.
// CHECK: [[idx:%[0-9]+]] = OpLoad %uint %gl_LocalInvocationIndex
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Output_bool [[var]] [[idx]]
// CHECK: OpStore [[ac]] %false
    primitives[svGroupIndex].m_cullPrimitive = false;
}
