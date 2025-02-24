// RUN: %dxc /Tcs_6_0 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: entry

#define MAX_INDEX 14

groupshared float g_Array[2][(MAX_INDEX * MAX_INDEX)];
RWStructuredBuffer<float4> output;

[numthreads(1,1,1)] void main(uint GroupIndex
                                : SV_GroupIndex) {
  uint idx;
  for (idx = 0; idx < (MAX_INDEX * MAX_INDEX); idx++) {
    g_Array[GroupIndex][idx] = 0.0f;
  }

  output[GroupIndex] = float4(g_Array[GroupIndex][0], g_Array[GroupIndex][1], g_Array[GroupIndex][2], g_Array[GroupIndex][3]);
}