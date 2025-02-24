// RUN: %dxc /Tcs_6_0 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: entry

#define MAX_INDEX 5

groupshared float g_Array[2][(MAX_INDEX * MAX_INDEX)];
RWStructuredBuffer<float4> output;

[numthreads(1,1,1)] void main(uint GroupIndex
                                : SV_GroupIndex) {
  uint idx;
  float l_Array[(MAX_INDEX * MAX_INDEX)] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  for (idx = 0; idx < (MAX_INDEX * MAX_INDEX); idx++) {
    g_Array[GroupIndex][idx] = l_Array[idx];
  }

  output[GroupIndex] = float4(g_Array[GroupIndex][0], g_Array[GroupIndex][1], g_Array[GroupIndex][2], g_Array[GroupIndex][3]);
}