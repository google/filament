// RUN: %dxc -E main -T ps_6_0  %s | FileCheck %s

// CHECK: define void @main()
// CHECK: %{{[a-z0-9]+}} = shl i32 %{{[a-z0-9]+}}, 2
// CHECK: %{{[a-z0-9]+}} = and i32 %{{[a-z0-9]+}}, 8
// CHECK: %{{[a-z0-9]+}} = or i32 %{{[a-z0-9]+}}, 20
// CHECK: %{{[a-z0-9]+}} = getelementptr inbounds [4 x float], [4 x float]* %{{[a-z0-9]+}}, i32 0, i32 %{{[a-z0-9]+}}
// CHECK: %{{[a-z0-9]+}} = getelementptr inbounds [4 x float], [4 x float]* %{{[a-z0-9]+}}, i32 0, i32 %{{[a-z0-9]+}}
// CHECK: entry

float4x4 buf;
float main(uint a : A) : SV_Target {
 return buf[a >> 2][(((a & 3) | 5) * 4)] * buf[2][a << 2];
}