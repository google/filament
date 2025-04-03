// RUN: %dxc -T ps_6_0 -E main -Vd -spirv %s -spirv | FileCheck %s

template<class T, class U>
[[vk::ext_instruction(/*spv::OpBitcast*/124)]]
T Bitcast(U);

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 16
// CHECK: %S = OpTypeStruct %v4float %v4float

struct S
{
    [[vk::ext_decorate(/*offset*/ 35, 0)]] float4 f1;
    [[vk::ext_decorate(/*offset*/ 35, 16)]] float4 f2;
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

float4 main() : SV_TARGET
{

// CHECK: [[BC:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_S {{%[0-9]+}}
  PointerType ptr = Bitcast<PointerType>(address);

// CHECK: [[LD:%[0-9]+]] = OpLoad %S [[BC]] Aligned 32
// CHECK: [[RET:%[0-9]+]] = OpCompositeExtract %v4float [[LD]] 0
// CHECK: OpStore %out_var_SV_TARGET [[RET]]
  return Load(ptr).f1;
}
