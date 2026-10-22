// RUN: %dxc -spirv -T lib_6_8 %s -fcgl | FileCheck %s

enum LuminairePlanesSymmetry : uint
{
    ISOTROPIC = 0u,
    NO_LATERAL_SYMMET
};

using symmetry_t = LuminairePlanesSymmetry;
const symmetry_t symmetry;

[numthreads(1,1,1)]
[shader("compute")]
void main(uint3 id : SV_DispatchThreadID)
{
// CHECK: [[AC:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %_Globals %int_0
// CHECK: [[LHS:%[0-9]+]] = OpLoad %int [[AC]]
// CHECK: [[RHS:%[0-9]+]] = OpLoad %int %ISOTROPIC
// CHECK: OpIEqual %bool [[LHS]] [[RHS]]
  const uint i0 = (symmetry == symmetry_t::ISOTROPIC) ? 0u : 1u;
}
