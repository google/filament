// RUN: %dxc /Tps_6_0 /Eps_main  %s | FileCheck %s
// CHECK: define void @ps_main()
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0x3FFB333340000000)
// CHECK: entry


float ps_main() : SV_Target
{
  float ip1;
  float fp1 = modf(-1.3, ip1);
  float ip2;
  float fp2 = modf(2.5, ip2);
  // expression: 2.0+(-0.3) = 1.7 or 0x3FFB333340000000
  return ip2 + fp1;
}