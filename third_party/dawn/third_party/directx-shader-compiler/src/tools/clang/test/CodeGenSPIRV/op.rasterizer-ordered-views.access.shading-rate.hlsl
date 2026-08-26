// RUN: %dxc -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main ShadingRateInterlockOrderedEXT
// CHECK-NOT: OpExecutionMode %main PixelInterlockOrderedEXT
// CHECK-NOT: OpExecutionMode %main SampleInterlockOrrderedEXT
// CHECK-NOT: OpExecutionMode %main ShadingRateInterlockOrderedEXT

RasterizerOrderedBuffer<float> rovBuf;

void main(uint rate: SV_ShadingRate) {
  float val;

// CHECK: OpBeginInvocationInterlockEXT
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpImageRead
// CHECK-NEXT: OpEndInvocationInterlockEXT
  val = rovBuf[0];
}
