// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure this fails
// CHECK: error: local resource not guaranteed to map to unique global resource

RWBuffer<float4> BufArray[2][2][3];
RWBuffer<float4> Buf2;

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
          bufarr[i%2][j%3] = Buf2;  // Illegal: assign different global resource
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