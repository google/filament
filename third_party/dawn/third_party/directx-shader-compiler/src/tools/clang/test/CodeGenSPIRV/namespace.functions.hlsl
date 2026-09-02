// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpName %AddRed "AddRed"
// CHECK: OpName %A__AddRed "A::AddRed"
// CHECK: OpName %A__B__AddRed "A::B::AddRed"
// CHECK: OpName %A__B__AddBlue "A::B::AddBlue"
// CHECK: OpName %A__AddGreen "A::AddGreen"
// CHECK: OpName %A__createMyStruct "A::createMyStruct"
// CHECK: OpName %A__myStruct_add "A::myStruct.add"

// CHECK: [[v3f2:%[0-9]+]] = OpConstantComposite %v3float %float_2 %float_2 %float_2
// CHECK: [[v4f0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK: [[v3f0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK: [[v3f1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1
// CHECK: [[v3f3:%[0-9]+]] = OpConstantComposite %v3float %float_3 %float_3 %float_3

namespace A {

  float3 AddRed() { return float3(0, 0, 0); }
  float3 AddGreen();

  namespace B {
    typedef float3 f3;
    float3 AddRed() { return float3(1, 1, 1); }
    float3 AddBlue();
  }  // end namespace B

  struct myStruct {
    int point1;
    int point2;
    int add() {
      return point1 + point2;
    }
  };
  
  myStruct createMyStruct() {
    myStruct s;
    return s;
  }
}  // end namespace A


float3 AddRed() { return float3(2, 2, 2); }

float4 main(float4 PosCS : SV_Position) : SV_Target
{
// CHECK: {{%[0-9]+}} = OpFunctionCall %v3float %AddRed
  float3 val_1 = AddRed();
// CHECK: {{%[0-9]+}} = OpFunctionCall %v3float %A__AddRed
  float3 val_2 = A::AddRed();
// CHECK: {{%[0-9]+}} = OpFunctionCall %v3float %A__B__AddRed
  float3 val_3 = A::B::AddRed();

// CHECK: {{%[0-9]+}} = OpFunctionCall %v3float %A__B__AddBlue
  float3 val_4 = A::B::AddBlue();
// CHECK: {{%[0-9]+}} = OpFunctionCall %v3float %A__AddGreen
  float3 val_5 = A::AddGreen();

// CHECK: OpStore %vec3f [[v3f2]]
  A::B::f3 vec3f = float3(2,2,2);

// CHECK: [[s:%[0-9]+]] = OpFunctionCall %myStruct %A__createMyStruct
// CHECK: OpStore %s [[s]]
  A::myStruct s = A::createMyStruct();
// CHECK: {{%[0-9]+}} = OpFunctionCall %int %A__myStruct_add %s
  int val_6 = s.add();

  return float4(0,0,0,0);
}

float3 A::B::AddBlue() { return float3(1, 1, 1); }
float3 A::AddGreen() { return float3(3, 3, 3); }

// CHECK: %AddRed = OpFunction %v3float None
// CHECK: OpReturnValue [[v3f2]]

// CHECK: %A__AddRed = OpFunction %v3float None
// CHECK: OpReturnValue [[v3f0]]

// CHECK: %A__B__AddRed = OpFunction %v3float None
// CHECK: OpReturnValue [[v3f1]]

// CHECK: %A__B__AddBlue = OpFunction %v3float None
// CHECK: OpReturnValue [[v3f1]]

// CHECK: %A__AddGreen = OpFunction %v3float None
// CHECK: OpReturnValue [[v3f3]]

// TODO: struct name should also be updated to A::myStruct
// CHECK: %A__createMyStruct = OpFunction %myStruct None

// CHECK: %A__myStruct_add = OpFunction %int None
// CHECK: %param_this = OpFunctionParameter %_ptr_Function_myStruct
// CHECK: OpAccessChain %_ptr_Function_int %param_this %int_0
// CHECK: OpAccessChain %_ptr_Function_int %param_this %int_1
