// RUN: %dxc -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main PixelInterlockOrderedEXT
// CHECK-NOT: OpExecutionMode %main PixelInterlockOrderedEXT
// CHECK-NOT: OpExecutionMode %main SampleInterlockOrderedEXT
// CHECK-NOT: OpExecutionMode %main ShadingRateInterlockOrderedEXT

struct S {
    float f;
};

SamplerState gSampler;

RasterizerOrderedBuffer<float> rovBuf;
RasterizerOrderedByteAddressBuffer rovBABuf;
RasterizerOrderedStructuredBuffer<S> rovSBuf;
RasterizerOrderedTexture1D<float> rovTex1D;
RasterizerOrderedTexture1DArray<float> rovTex1DArray;
RasterizerOrderedTexture2D<float> rovTex2D;
RasterizerOrderedTexture2DArray<float> rovTex2DArray;
RasterizerOrderedTexture3D<float> rovTex3D;

void main() {
  float val;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovBuf[0];
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageWrite
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovBuf[0] = 123.0;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageWrite
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovBuf[0]++;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpAccessChain
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovBABuf.Load(0);
// CHECK: OpBeginInvocationInterlockEXT
// CHECK: OpAccessChain
// CHECK: OpStore
// CHECK: OpEndInvocationInterlockEXT
  rovBABuf.Store(0, 123.0);

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovSBuf[0].f;
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpStore
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovSBuf[0].f = 123.0;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovTex1D[0];
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageWrite
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovTex1D[0] = 123.0;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovTex1DArray[uint2(0,0)];
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageWrite
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovTex1DArray[uint2(0,0)] = 123.0;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovTex2D[uint2(0,0)];
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageWrite
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovTex2D[uint2(0,0)] = 123.0;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovTex2DArray[uint3(0,0,0)];
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageWrite
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovTex2DArray[uint3(0,0,0)] = 123.0;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovTex3D[uint3(0,0,0)];
// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageWrite
// CHECK-NEXT: OpEndInvocationInterlockEXT
  rovTex3D[uint3(0,0,0)] = 123.0;
}
