// RUN: %dxilver 1.1 | %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK-NOT: Typed UAV Load Additional Formats

RWBuffer<int> buf[5][1];
RWBuffer<int2> buf_2[3];
int ref;

[numthreads(64,1,1)]
void main(uint tid : SV_DispatchThreadID)
{
  if (ref > 0)
    buf[1][0][1] = 3;
  else
    buf_2[2][3] = 3;
}