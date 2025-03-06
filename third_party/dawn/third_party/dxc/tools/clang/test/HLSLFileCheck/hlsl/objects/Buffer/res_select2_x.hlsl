// RUN: %dxc -T lib_6_x -auto-binding-space 11 %s | FileCheck %s

// lib_6_x does not reduce phi/select of resource or handle in lib.
// CHECK: phi %dx.types.Handle
// CHECK: select i1 %{{[^,]+}}, %dx.types.Handle
// CHECK: ret <4 x float>

RWBuffer<float4> BufArray[2][2][3];

float4 test(int i, int j, int m) {
  RWBuffer<float4> a = BufArray[m][0][0];
  RWBuffer<float4> b = BufArray[0][m][1];
  RWBuffer<float4> c = BufArray[0][1][m];
  RWBuffer<float4> bufarr[2][3] = BufArray[m];

  RWBuffer<float4> buf = c;
  while (i > 9) {
     while (j < 4) {
        if (i < m)
          buf = b;
        else
          bufarr[i%2][j%3] = buf;
        buf[j] = i;
        j++;
     }
     if (m > j)
       buf = a;
     buf[m] = i;
     i--;
  }
  buf[i] = j;
  bufarr[m%2][j%3][j] = m;
  return j;
}