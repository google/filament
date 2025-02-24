// RUN: %dxc -T lib_6_5 %s  | FileCheck %s

// Make sure we don't use alloca, meaning we successfully aliased the static
// and local resource arrays to the original global array.

// CHECK: define void @main()
// CHECK-NOT: alloca

Texture2D<float4> buf[16];
static Texture2D<float4> s_buf[16];

void Init(Texture2D<float4> buf[16]) {
  s_buf = buf;
}

float4 Use(Texture2D<float4> buf[16], uint i) {
  return buf[i][int2(3,4)];
}

float4 Use(uint i) {
  return Use(s_buf, i);
}

[shader("pixel")]
float4 main(uint i:I) : SV_Target {
  Texture2D<float4> lbuf[16];
  lbuf = buf;
  Init(lbuf);
  return Use(i);
}

[shader("pixel")]
float4 other_main(uint i:I) : SV_Target {
  Texture2D<float4> lbuf[16];
  lbuf = buf;
  Init(lbuf);
  return Use(i);
}
