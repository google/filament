// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpMemberDecorate %OuterStruct 0 Offset 0
// CHECK: OpMemberDecorate %OuterStruct 1 Offset 0
// CHECK: OpMemberDecorate %type_Params_cbuffer 0 Offset 0

// CHECK:        %InnerStruct0 = OpTypeStruct
// CHECK:        %InnerStruct1 = OpTypeStruct
// CHECK:         %OuterStruct = OpTypeStruct %InnerStruct0 %InnerStruct1
// CHECK: %type_Params_cbuffer = OpTypeStruct %OuterStruct

struct InnerStruct0 {};

struct InnerStruct1 {};

struct OuterStruct
{
  InnerStruct0 s0;
  InnerStruct1 s1;
};

struct MyParams
{
  OuterStruct constants;
};

cbuffer Params_cbuffer : register(b0)
{
  OuterStruct constants;
};

MyParams CreateParams()
{
  MyParams i;
  i.constants = constants;
  return i;
}

static const MyParams params_vars = CreateParams();

float4 main(float4 pos : POSITION) : SV_POSITION
{
  return float4(0, 0, 0, 0);
}
