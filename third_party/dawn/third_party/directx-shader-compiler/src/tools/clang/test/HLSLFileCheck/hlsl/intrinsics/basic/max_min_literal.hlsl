// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: FMax
// CHECK-NOT: FMin
// CHECK-NOT: IMax
// CHECK-NOT: IMin
// CHECK-NOT: UMax
// CHECK-NOT: UMin


#define FA float4(0.0f, 1.0f, 2.0f, 3.0f)
#define FB float4(4.0f, 5.0f, 6.0f, 7.0f)
#define IA int4(0, 1, 2, 3)
#define IB int4(4, 5, 6, 7)
#define UA uint4(0, 1, 2, 3)
#define UB uint4(4, 5, 6, 7)


float4 main(float4 a : A) : SV_TARGET
{
  return max(FA,FB) + min(FA,FB) + max(IA,IB) + min(IA,IB) + max(UA,UB) + min(UA,UB);
}

