// RUN: %dxc -T cs_6_6 -E MainRayGenShader -fcgl -spirv %s | FileCheck %s --implicit-check-not=OpTypeRayQueryKHR

// A user-defined struct named "RayQuery" that shadows the reserved intrinsic
// name must be lowered as an ordinary struct, not as the opaque ray query
// type. Previously this crashed the SPIR-V backend.
// See https://github.com/microsoft/DirectXShaderCompiler/issues/8601

namespace UnifiedRT {
struct RayQuery {
  float4 foo;
};
} // namespace UnifiedRT

// The shadowing struct is lowered to an ordinary struct with a float4 field,
// and the local variable uses that struct type in the Function storage class.
// CHECK: %RayQuery = OpTypeStruct %v4float
// CHECK: %_ptr_Function_RayQuery = OpTypePointer Function %RayQuery
// CHECK: %rayQuery = OpVariable %_ptr_Function_RayQuery Function

// It must not be lowered to the opaque ray query type. The absence of
// OpTypeRayQueryKHR anywhere in the module is checked by the
// --implicit-check-not on the FileCheck invocation above.

StructuredBuffer<uint> _UnifiedRT_DispatchDims;

[numthreads(128, 1, 1)]
void MainRayGenShader(in uint3 gidx : SV_DispatchThreadID,
                      in uint lidx : SV_GroupIndex) {
  if (gidx.x >= _UnifiedRT_DispatchDims[0] ||
      gidx.y >= _UnifiedRT_DispatchDims[1] ||
      gidx.z >= _UnifiedRT_DispatchDims[2])
    return;

  UnifiedRT::RayQuery rayQuery;
  rayQuery.foo = float4(1.0, 1.0, 1.0, 1.0);
}
