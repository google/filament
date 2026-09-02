// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Note that since 'derived_count' is static,
// Derived structures are in fact empty (have no members other than their Base)
// The layout calculations should *not* consider Derived as size 0.

// CHECK: OpDecorate %_arr_v4float_uint_4 ArrayStride 16
// CHECK: OpMemberDecorate %Base 0 Offset 0
struct Base {
	static const uint count = 4;
	float4 arr[count];
};

// CHECK: OpMemberDecorate %Derived 0 Offset 0
struct Derived : Base {
  static const uint derived_count = Base::count;
};

// CHECK: OpDecorate %_runtimearr_Derived ArrayStride 64
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_Derived 0 Offset 0
RWStructuredBuffer<Derived> rwsb : register(u0);

[numthreads(64u, 1u, 1u)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
}
