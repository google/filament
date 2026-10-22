// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s -check-prefix=RELAXED
// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv -fvk-use-gl-layout | FileCheck %s -check-prefix=RELAXED
// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv -fvk-use-scalar-layout | FileCheck %s -check-prefix=SCALAR

struct empty_struct { };


// RELAXED: OpMemberDecorate %type_cb 0 Offset 0
// RELAXED: OpMemberDecorate %type_cb 1 Offset 16
// RELAXED: OpMemberDecorate %type_cb 2 Offset 16
// RELAXED: OpMemberDecorate %type_cb 3 Offset 32

// SCALAR: OpMemberDecorate %type_cb 0 Offset 0
// SCALAR: OpMemberDecorate %type_cb 1 Offset 12
// SCALAR: OpMemberDecorate %type_cb 2 Offset 12
// SCALAR: OpMemberDecorate %type_cb 3 Offset 20

cbuffer cb
{
	float3 a;
	empty_struct b;
	float2 c;

	float4 test;
};

float4 main() : SV_TARGET
{
	return test;
}
