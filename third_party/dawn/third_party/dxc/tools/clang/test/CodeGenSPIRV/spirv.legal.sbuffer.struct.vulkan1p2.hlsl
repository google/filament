// RUN: %dxc -T cs_6_3 -fspv-target-env=vulkan1.2 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %Struct = OpTypeStruct %v3float %_ptr_StorageBuffer_type_StructuredBuffer_uint
struct Struct
{
  float3 foo;
  StructuredBuffer<uint> buffer;
};

// CHECK: %Struct2 = OpTypeStruct %v3float %_ptr_StorageBuffer_type_StructuredBuffer_uint %_ptr_StorageBuffer_type_StructuredBuffer_uint
struct Struct2
{
  float3 foo;
  StructuredBuffer<uint> buffer1;
  StructuredBuffer<uint> buffer2;
};

// CHECK: [[fn1:%[0-9]+]] = OpTypeFunction %Struct
// CHECK: %g_stuff_buffer = OpVariable %_ptr_StorageBuffer_type_StructuredBuffer_uint StorageBuffer
// CHECK: %g_output = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_uint StorageBuffer
StructuredBuffer<uint> g_stuff_buffer;
RWStructuredBuffer<uint> g_output;

Struct make_struct()
{
  Struct s;
  s.buffer = g_stuff_buffer;
  return s;
}

[numthreads(1, 1, 1)]
void main()
{
// CHECK: %s = OpVariable %_ptr_Function_Struct Function
  Struct s = make_struct();

// CHECK: %s2 = OpVariable %_ptr_Function_Struct2 Function
  Struct2 s2;
  s2.buffer1 = g_stuff_buffer;
  s2.buffer2 = g_stuff_buffer;

// CHECK: [[p1:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_StorageBuffer_type_StructuredBuffer_uint %s %int_1
// CHECK: [[p2:%[0-9]+]] = OpLoad %_ptr_StorageBuffer_type_StructuredBuffer_uint [[p1]]
// CHECK: [[p3:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_uint [[p2]] %int_0 %uint_0
// CHECK: [[p4:%[0-9]+]] = OpLoad %uint [[p3]]
// CHECK: [[p5:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_uint %g_output %int_0 %uint_0
// CHECK:               OpStore [[p5]] [[p4]]
  g_output[0] = s.buffer[0];
}

// CHECK: %make_struct = OpFunction %Struct None [[fn1]]
