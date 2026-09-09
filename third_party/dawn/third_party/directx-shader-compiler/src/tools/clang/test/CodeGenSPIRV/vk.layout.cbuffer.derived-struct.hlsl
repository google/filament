// RUN: %dxc -T vs_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

struct VertexInput {
  float4 position : POSITION;
};

struct PixelOutput {
  float4 position : SV_POSITION;
};

// CHECK: OpMemberDecorate %Base1 0 Offset 0
struct Base1 {
  float4 foo1;
};

// CHECK: OpMemberDecorate %Base2 0 Offset 0
// CHECK: OpMemberDecorate %Base2 1 Offset 16
struct Base2 : Base1 {
  float4 foo2;
};

// CHECK: OpMemberDecorate %Derived 0 Offset 0
// CHECK: OpMemberDecorate %Derived 1 Offset 32
// CHECK: OpMemberDecorate %Derived 2 Offset 48
struct Derived : Base2 {
  float4 foo3;
  float4 foo4;
};

// CHECK: OpMemberDecorate %type_constantData 0 Offset 0
// CHECK: OpMemberDecorate %type_constantData 1 Offset 64
cbuffer constantData : register(b0) {
  Derived derivedData;
  float4x4 MVP;
}


// CHECK:             %Base1 = OpTypeStruct %v4float
// CHECK:             %Base2 = OpTypeStruct %Base1 %v4float
// CHECK:           %Derived = OpTypeStruct %Base2 %v4float %v4float
// CHECK: %type_constantData = OpTypeStruct %Derived %mat4v4float

PixelOutput main(const VertexInput vertex) {
  PixelOutput pixel;
  pixel.position = mul(vertex.position, MVP);

  return pixel;
}

