// REQUIRES: dxil-1-9
// RUN: %dxc -T cs_6_6 -DSCALAR -DFUNC=ddx %s        | FileCheck %s
// RUN: %dxc -T cs_6_6 -DSCALAR -DFUNC=ddx_coarse %s | FileCheck %s
// RUN: %dxc -T cs_6_6 -DSCALAR -DFUNC=ddx_fine %s   | FileCheck %s
// RUN: %dxc -T cs_6_6 -DSCALAR -DFUNC=ddy %s        | FileCheck %s
// RUN: %dxc -T cs_6_6 -DSCALAR -DFUNC=ddy_coarse %s | FileCheck %s
// RUN: %dxc -T cs_6_6 -DSCALAR -DFUNC=ddy_fine %s   | FileCheck %s
// RUN: %dxc -T cs_6_9 -DFUNC=ddx %s                  | FileCheck %s
// RUN: %dxc -T cs_6_9 -DFUNC=ddx_coarse %s           | FileCheck %s
// RUN: %dxc -T cs_6_9 -DFUNC=ddx_fine %s             | FileCheck %s
// RUN: %dxc -T cs_6_9 -DFUNC=ddy %s                  | FileCheck %s
// RUN: %dxc -T cs_6_9 -DFUNC=ddy_coarse %s           | FileCheck %s
// RUN: %dxc -T cs_6_9 -DFUNC=ddy_fine %s             | FileCheck %s
// RUN: %dxc -T cs_6_6 -DSCALAR -DFUNC=ddx -print-before hlsl-dxilfinalize %s 2>&1 | FileCheck %s --check-prefix=PRE-FINALIZE
// RUN: %dxc -T cs_6_6 -fcgl -DSCALAR -DFUNC=ddx %s        | FileCheck %s --check-prefixes=HL,HL-DDX
// RUN: %dxc -T cs_6_6 -fcgl -DSCALAR -DFUNC=ddx_coarse %s | FileCheck %s --check-prefixes=HL,HL-DDX-COARSE
// RUN: %dxc -T cs_6_6 -fcgl -DSCALAR -DFUNC=ddx_fine %s   | FileCheck %s --check-prefixes=HL,HL-DDX-FINE
// RUN: %dxc -T cs_6_6 -fcgl -DSCALAR -DFUNC=ddy %s        | FileCheck %s --check-prefixes=HL,HL-DDY
// RUN: %dxc -T cs_6_6 -fcgl -DSCALAR -DFUNC=ddy_coarse %s | FileCheck %s --check-prefixes=HL,HL-DDY-COARSE
// RUN: %dxc -T cs_6_6 -fcgl -DSCALAR -DFUNC=ddy_fine %s   | FileCheck %s --check-prefixes=HL,HL-DDY-FINE

RWByteAddressBuffer output;

[numthreads(2, 2, 1)]
void main() {
  uint laneIndex = WaveGetLaneIndex();

#ifdef SCALAR
  float value = float(laneIndex * 2);
  float result = FUNC(value);
#else
  vector<float, 3> value = 1.0;
  value += float(laneIndex * 2);
  vector<float, 3> result = FUNC(value);
#endif

  // Derivatives require all quad lanes, so the call must remain before the
  // divergent branch even though only lane 3 consumes the result.
  // CHECK: call {{(<3 x float>|float)}} @dx.op.unary{{.*}}(i32 {{8[3-6]}}, {{.*}})
  // CHECK: icmp eq i32
  // CHECK-NEXT: br i1
  if (laneIndex == 3) {
#ifdef SCALAR
    output.Store(0, asuint(result));
#else
    output.Store<vector<float, 3> >(0, result);
#endif
  }
}

// CHECK-NOT: convergent

// PRE-FINALIZE: call float @dx.op.unary.f32(i32 83, {{.*}}) #[[DXIL_CONV:[0-9]+]]
// PRE-FINALIZE: attributes #[[DXIL_CONV]] = { convergent }

// HL-DDX: call float @"dx.hl.op.cvrn.float (i32, float)"(i32 125,
// HL-DDX-COARSE: call float @"dx.hl.op.cvrn.float (i32, float)"(i32 126,
// HL-DDX-FINE: call float @"dx.hl.op.cvrn.float (i32, float)"(i32 127,
// HL-DDY: call float @"dx.hl.op.cvrn.float (i32, float)"(i32 128,
// HL-DDY-COARSE: call float @"dx.hl.op.cvrn.float (i32, float)"(i32 129,
// HL-DDY-FINE: call float @"dx.hl.op.cvrn.float (i32, float)"(i32 130,
// HL: ; Function Attrs: convergent nounwind readnone
// HL-NEXT: declare float @"dx.hl.op.cvrn.float (i32, float)"(i32, float)
