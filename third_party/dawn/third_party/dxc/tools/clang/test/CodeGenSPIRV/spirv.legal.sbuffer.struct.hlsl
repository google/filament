// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Basic {
    float3 a;
    float4 b;
};

// CHECK: %S = OpTypeStruct %_ptr_Uniform_type_AppendStructuredBuffer_v4float %_ptr_Uniform_type_ConsumeStructuredBuffer_v4float
struct S {
     AppendStructuredBuffer<float4> append;
    ConsumeStructuredBuffer<float4> consume;
};

// CHECK: %T = OpTypeStruct %_ptr_Uniform_type_StructuredBuffer_Basic %_ptr_Uniform_type_RWStructuredBuffer_Basic
struct T {
      StructuredBuffer<Basic> ro;
    RWStructuredBuffer<Basic> rw;

};

struct U {
    StructuredBuffer<Basic> basic;
    StructuredBuffer<int>   integer;

    StructuredBuffer<Basic> getSBufferStruct() { return basic;   }
    StructuredBuffer<int>   getSBufferInt()    { return integer; }
};

// CHECK: %Combine = OpTypeStruct %S %T %_ptr_Uniform_type_ByteAddressBuffer %_ptr_Uniform_type_RWByteAddressBuffer %U
struct Combine {
                      S s;
                      T t;
      ByteAddressBuffer ro;
    RWByteAddressBuffer rw;
                      U u;

    U getU() { return u; }
};

       StructuredBuffer<Basic>  gSBuffer;
     RWStructuredBuffer<Basic>  gRWSBuffer;
 AppendStructuredBuffer<float4> gASBuffer;
ConsumeStructuredBuffer<float4> gCSBuffer;
      ByteAddressBuffer         gBABuffer;
    RWByteAddressBuffer         gRWBABuffer;

float4 foo(Combine comb);

float4 main() : SV_Target {
    Combine c;

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_AppendStructuredBuffer_v4float %c %int_0 %int_0
// CHECK-NEXT:                OpStore [[ptr]] %gASBuffer
    c.s.append = gASBuffer;
// CHECK:      [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_ConsumeStructuredBuffer_v4float %c %int_0 %int_1
// CHECK-NEXT:                OpStore [[ptr_0]] %gCSBuffer
    c.s.consume = gCSBuffer;

    T t;
// CHECK:      [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_StructuredBuffer_Basic %t %int_0
// CHECK-NEXT:                OpStore [[ptr_1]] %gSBuffer
    t.ro = gSBuffer;
// CHECK:      [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_RWStructuredBuffer_Basic %t %int_1
// CHECK-NEXT:                OpStore [[ptr_2]] %gRWSBuffer
    t.rw = gRWSBuffer;
// CHECK:      [[val:%[0-9]+]] = OpLoad %T %t
// CHECK-NEXT: [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Function_T %c %int_1
// CHECK-NEXT:                OpStore [[ptr_3]] [[val]]
    c.t = t;

// CHECK:      [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer %c %int_2
// CHECK-NEXT:                OpStore [[ptr_4]] %gBABuffer
    c.ro = gBABuffer;
// CHECK:      [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_RWByteAddressBuffer %c %int_3
// CHECK-NEXT:                OpStore [[ptr_5]] %gRWBABuffer
    c.rw = gRWBABuffer;

// Make sure that we create temporary variable for intermediate objects since
// the function expect pointers as parameters.
// CHECK:     [[call1:%[0-9]+]] = OpFunctionCall %U %Combine_getU %c
// CHECK-NEXT:                 OpStore %temp_var_U [[call1]]
// CHECK-NEXT:[[call2:%[0-9]+]] = OpFunctionCall %_ptr_Uniform_type_StructuredBuffer_Basic %U_getSBufferStruct %temp_var_U
// CHECK-NEXT:  [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[call2]] %int_0 %uint_10 %int_1
// CHECK-NEXT:      {{%[0-9]+}} = OpLoad %v4float [[ptr_6]]
    float4 val = c.getU().getSBufferStruct()[10].b;

// Check StructuredBuffer of scalar type
// CHECK:     [[call2_0:%[0-9]+]] = OpFunctionCall %_ptr_Uniform_type_StructuredBuffer_int %U_getSBufferInt %temp_var_U_0
// CHECK-NEXT:      {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_int [[call2_0]] %int_0 %uint_42
    int index = c.getU().getSBufferInt()[42];

// CHECK:      [[val_0:%[0-9]+]] = OpLoad %Combine %c
// CHECK:                     OpStore %param_var_comb [[val_0]]
    return foo(c);
}
float4 foo(Combine comb) {
// CHECK:      [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer %comb %int_2
// CHECK-NEXT: [[ptr2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ByteAddressBuffer [[ptr1]]
// CHECK-NEXT:  [[idx:%[0-9]+]] = OpShiftRightLogical %uint %uint_5 %uint_2
// CHECK-NEXT:      {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_uint [[ptr2]] %uint_0 [[idx]]
    uint val = comb.ro.Load(5);

// CHECK:      [[ptr1_0:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_StructuredBuffer_Basic %comb %int_1 %int_0
// CHECK-NEXT: [[ptr2_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_StructuredBuffer_Basic [[ptr1_0]]
// CHECK-NEXT:      {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_v4float [[ptr2_0]] %int_0 %uint_0 %int_1
    return comb.t.ro[0].b;
}
