// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 1, outputs: 0
// CHECK: Outputs dependent on ViewId: {  }
// CHECK: Inputs contributing to computation of Outputs:

RWBuffer<uint> buf1;

void main(uint idx : IDX, uint vid : SV_ViewID)
{
  buf1[idx] = vid;
}