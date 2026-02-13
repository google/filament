// RUN: %dxc -T ps_6_0 -E main -Vd -spirv %s -spirv | FileCheck %s

template<class T, class U>
[[vk::ext_instruction(/*spv::OpBitcast*/124)]]
T Bitcast(U);

// CHECK-DAG: OpMemberDecorate %S 0 Offset 0
// CHECK-DAG: OpMemberDecorate %S 1 Offset 16
// CHECK-DAG: %S = OpTypeStruct %v4float %v4float

struct S
{
    [[vk::ext_decorate(/*offset*/ 35, 0)]] float4 f1;
    [[vk::ext_decorate(/*offset*/ 35, 16)]] float4 f2;
};

// CHECK-DAG: OpDecorateString %out_var_SV_TARGET UserSemantic "raster_order_group_0"
struct PixelOutput
{
	[[vk::location(0), vk::ext_decorate_string(5635, "raster_order_group_0")]] float4 rt0 : SV_TARGET;
};

using PointerType = vk::SpirvOpaqueType<
    /* OpTypePointer */ 32,
    /* PhysicalStorageBuffer */ vk::Literal<vk::integral_constant<uint,5349> >,
    S>;

[[vk::ext_capability(/*PhysicalStorageBufferAddresses */ 5347 )]]
[[vk::ext_instruction( /*OpLoad*/ 61 )]]
S Load(PointerType pointer,
       [[vk::ext_literal]] uint32_t __aligned=/*Aligned*/0x00000002,
       [[vk::ext_literal]] uint32_t __alignment=32);

uint64_t address;

PixelOutput main()
{

// CHECK: [[BC:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_S {{%[0-9]+}}
  PointerType ptr = Bitcast<PointerType>(address);

PixelOutput output;
// CHECK: [[LD:%[0-9]+]] = OpLoad %S [[BC]] Aligned 32
// CHECK: [[RET:%[0-9]+]] = OpCompositeExtract %v4float [[LD]] 0
// CHECK: OpStore %out_var_SV_TARGET [[RET]]
output.rt0 = Load(ptr).f1;
  return output;
}
