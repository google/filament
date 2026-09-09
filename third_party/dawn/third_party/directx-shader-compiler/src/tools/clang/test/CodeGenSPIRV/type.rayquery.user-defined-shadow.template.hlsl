// RUN: %dxc -T cs_6_6 -E MainRayGenShader -HV 2021 -fcgl -spirv %s | FileCheck %s --implicit-check-not=OpTypeRayQueryKHR

// A user-defined class template named "RayQuery" that shadows the reserved
// intrinsic name must be lowered as an ordinary struct, not as the opaque ray
// query type. The intrinsic is itself a class template, so detecting it by
// name and specialization kind alone also misclassifies this case.
// See https://github.com/microsoft/DirectXShaderCompiler/issues/8601

namespace UnifiedRT {
template <typename T> struct RayQuery {
  T foo;
};
} // namespace UnifiedRT

// The shadowing template instance is lowered to an ordinary struct with a
// float4 field, and the local variable uses that struct type in the Function
// storage class.
// CHECK: %RayQuery = OpTypeStruct %v4float
// CHECK: %_ptr_Function_RayQuery = OpTypePointer Function %RayQuery
// CHECK: %rayQuery = OpVariable %_ptr_Function_RayQuery Function

StructuredBuffer<uint> _UnifiedRT_DispatchDims;

[numthreads(128, 1, 1)]
void MainRayGenShader(in uint3 gidx : SV_DispatchThreadID,
                      in uint lidx : SV_GroupIndex) {
  if (gidx.x >= _UnifiedRT_DispatchDims[0] ||
      gidx.y >= _UnifiedRT_DispatchDims[1] ||
      gidx.z >= _UnifiedRT_DispatchDims[2])
    return;

  UnifiedRT::RayQuery<float4> rayQuery;
  rayQuery.foo = float4(1.0, 1.0, 1.0, 1.0);
}
