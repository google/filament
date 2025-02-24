// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// If column major, the CBuffer will have a size of 8 bytes, if row major it should be 20
// CHECK: CBuf; ; Offset: 0 Size: 20

typedef row_major float2x1 rmf2x1;
cbuffer CBuf { rmf2x1 mat; }
float main() : SV_Target { return mat._11; }