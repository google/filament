// RUN: %dxc /Tps_6_0 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: entry

typedef float3 TESTVECTORTYPEDEF[3];
typedef int TESTSCALARTYPEDEF[3];

float3 main(in TESTVECTORTYPEDEF a : A,
               in TESTSCALARTYPEDEF b : B) : SV_Target
{
  return a[0] + float3(b);
}