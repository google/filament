// RUN: %dxc -T cs_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

struct Base {
  uint base;
};

struct Derived : Base {
  uint a;
  uint b : 3;
  uint c : 3;
  uint d;
};

RWStructuredBuffer<Derived> g_probes : register(u0);

[numthreads(64u, 1u, 1u)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {

// CHECK:     [[p:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_Derived_0 Function
  Derived p;

// CHECK:   [[tmp:%[0-9]+]] = OpAccessChain %_ptr_Function_Base_0 [[p]] %uint_0
// CHECK:   [[tmp_0:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[tmp]] %int_0
// CHECK:                  OpStore [[tmp_0]] %uint_5
  p.base = 5;

// CHECK:   [[tmp_1:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[p]] %int_1
// CHECK:                  OpStore [[tmp_1]] %uint_1
  p.a = 1;

// CHECK:   [[tmp_2:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[p]] %int_2
// CHECK: [[value:%[0-9]+]] = OpLoad %uint [[tmp_2]]
// CHECK: [[value_0:%[0-9]+]] = OpBitFieldInsert %uint [[value]] %uint_2 %uint_0 %uint_3
// CHECK:                  OpStore [[tmp_2]] [[value_0]]
  p.b = 2;

// CHECK:   [[tmp_3:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[p]] %int_2
// CHECK: [[value_1:%[0-9]+]] = OpLoad %uint [[tmp_3]]
// CHECK: [[value_2:%[0-9]+]] = OpBitFieldInsert %uint [[value_1]] %uint_3 %uint_3 %uint_3
// CHECK:                  OpStore [[tmp_3]] [[value_2]]
  p.c = 3;

// CHECK:   [[tmp_4:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[p]] %int_3
// CHECK:                  OpStore [[tmp_4]] %uint_4
  p.d = 4;


// CHECK:     [[p_0:%[0-9]+]] = OpLoad %Derived_0 [[p]]
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_Derived %g_probes %int_0 %uint_0
// CHECK:   [[tmp_5:%[0-9]+]] = OpCompositeExtract %Base_0 [[p_0]] 0
// CHECK:   [[tmp_6:%[0-9]+]] = OpCompositeExtract %uint [[tmp_5]] 0
// CHECK:  [[base:%[0-9]+]] = OpCompositeConstruct %Base [[tmp_6]]
// CHECK:  [[mem1:%[0-9]+]] = OpCompositeExtract %uint [[p_0]] 1
// CHECK:  [[mem2:%[0-9]+]] = OpCompositeExtract %uint [[p_0]] 2
// CHECK:  [[mem3:%[0-9]+]] = OpCompositeExtract %uint [[p_0]] 3
// CHECK:   [[tmp_7:%[0-9]+]] = OpCompositeConstruct %Derived [[base]] [[mem1]] [[mem2]] [[mem3]]
// CHECK:                  OpStore [[ptr]] [[tmp_7]]
	g_probes[0] = p;
}

