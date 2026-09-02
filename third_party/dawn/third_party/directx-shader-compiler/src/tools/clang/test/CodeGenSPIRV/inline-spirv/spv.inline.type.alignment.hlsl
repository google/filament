// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

typedef vk::SpirvType</* OpTypeInt */ 21, /* size */ 1, /* alignment */ 1, vk::Literal<vk::integral_constant<uint, 8> >, vk::Literal<vk::integral_constant<bool, false> > > type1;
typedef vk::SpirvType</* OpTypeInt */ 21, /* size */ 1, /* alignment */ 32, vk::Literal<vk::integral_constant<uint, 8> >, vk::Literal<vk::integral_constant<bool, false> > > type2;
typedef vk::SpirvType</* OpTypeInt */ 21, /* size */ 32, /* alignment */ 1, vk::Literal<vk::integral_constant<uint, 8> >, vk::Literal<vk::integral_constant<bool, false> > > type3;

// CHECK: OpDecorate %_arr_spirvIntrinsicType_uint_3 ArrayStride 16
// CHECK: OpDecorate %_arr_spirvIntrinsicType_uint_3_0 ArrayStride 32

// CHECK: OpMemberDecorate %type__Globals 0 Offset 0
type1 a;

// CHECK: OpMemberDecorate %type__Globals 1 Offset 16
type1 a_arr[3];

// CHECK: OpMemberDecorate %type__Globals 2 Offset 64
type2 b;

// CHECK: OpMemberDecorate %type__Globals 3 Offset 96
type2 b_arr[3];

// CHECK: OpMemberDecorate %type__Globals 4 Offset 192
type3 c;

// CHECK: OpMemberDecorate %type__Globals 5 Offset 224
type3 c_arr[3];

// CHECK: OpMemberDecorate %type__Globals 6 Offset 320
int end;


// CHECK: %spirvIntrinsicType = OpTypeInt 8 0

[[vk::ext_capability(/* Int8 */ 39), vk::ext_capability(/* Int16 */ 22)]]
void main() {
}
