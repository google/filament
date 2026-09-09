// RUN: %dxc -T cs_6_0 -fdisable-loc-tracking %s 2>&1 | FileCheck %s

// CHECK: Function: main: note: Debug information is disabled which may impact diagnostic location accuracy. Re-run without -fdisable-loc-tracking to improve accuracy.
// CHECK-NOT: note: Debug information is disabled

RWBuffer<int> Buf[2];
RWBuffer<int> Out;

int getVal(bool B, int Idx) {
  RWBuffer<int> Local;
  if (B) Local = Buf[Idx];
  return Local[Idx];
}

int getValASecondTime(bool B, int Idx) {
  RWBuffer<int> Local;
  if (B) Local = Buf[Idx];
  return Local[Idx];
}

[numthreads(1,1,1)]
void main(uint GI : SV_GroupIndex) {
  Out[GI] = getVal(false, GI);
  Out[GI+1] = getValASecondTime(false, GI);
}
