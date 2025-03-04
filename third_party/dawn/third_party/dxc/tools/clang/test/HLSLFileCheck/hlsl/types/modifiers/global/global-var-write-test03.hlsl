// RUN: %dxc -T ps_6_0 -E PSMain /Gec -HV 2016 %s | FileCheck %s

// CHECK: define void @PSMain()
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 0)
// CHECK: %{{[a-z0-9]+.*[a-z0-9]*}} = fadd fast float %{{[a-z0-9]+.*[a-z0-9]*}}, 5.000000e+00
// CHECK: %{{[a-z0-9]+.*[a-z0-9]*}} = fmul fast float %{{[a-z0-9]+.*[a-z0-9]*}}, 3.000000e+00
// CHECK: ret void

float cbvar;

void AddVal() { cbvar += 5.0; }
void MulVal() { cbvar *= 3.0; }
float GetVal() { return cbvar; }

float PSMain() : SV_Target
{
  AddVal();
  MulVal();
  return GetVal();
}