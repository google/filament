// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: define void [[miss1:@"\\01\?miss1@[^\"]+"]](%struct.MyPayload* noalias nocapture %payload) #0 {
// CHECK:   %[[result:[^ ]+]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* %payload, i32 0, i32 0
// CHECK:   store <4 x float> <float 1.000000e+00, float 0.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %[[result]], align 4
// CHECK:   ret void

struct MyPayload {
  float4 color;
  uint2 pos;
};

[shader("miss")]
void miss1(inout MyPayload payload : SV_RayPayload)
{
  payload.color = float4(1.0, 0.0, 1.0, 1.0);
}
