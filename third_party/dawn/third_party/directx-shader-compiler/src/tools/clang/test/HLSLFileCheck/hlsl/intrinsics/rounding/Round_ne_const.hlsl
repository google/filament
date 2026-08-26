// RUN: %dxc -T vs_6_0 -E fr -DVAL=0.5  %s | %FileCheck -check-prefix=FLT_RND_1 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=-0.5  %s | %FileCheck -check-prefix=FLT_RND_2 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=1.5  %s | %FileCheck -check-prefix=FLT_RND_3 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=-1.5  %s | %FileCheck -check-prefix=FLT_RND_4 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=1.6  %s | %FileCheck -check-prefix=FLT_RND_5 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=1.3  %s | %FileCheck -check-prefix=FLT_RND_6 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=0.5 -HV 2016 %s | %FileCheck -check-prefix=FLT_RND_7 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=-0.5 -HV 2016 %s  | %FileCheck -check-prefix=FLT_RND_8 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=1.5 -HV 2016 %s  | %FileCheck -check-prefix=FLT_RND_9 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=-1.5 -HV 2016 %s  | %FileCheck -check-prefix=FLT_RND_10 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=1.6 -HV 2016  %s | %FileCheck -check-prefix=FLT_RND_11 %s
// RUN: %dxc -T vs_6_0 -E fr -DVAL=1.3 -HV 2016  %s | %FileCheck -check-prefix=FLT_RND_12 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=0.5  %s | %FileCheck -check-prefix=DBL_RND_1 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=-0.5  %s | %FileCheck -check-prefix=DBL_RND_2 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=1.5  %s | %FileCheck -check-prefix=DBL_RND_3 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=-1.5  %s | %FileCheck -check-prefix=DBL_RND_4 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=1.6  %s | %FileCheck -check-prefix=DBL_RND_5 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=1.3  %s | %FileCheck -check-prefix=DBL_RND_6 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=0.5 -HV 2016 %s | %FileCheck -check-prefix=DBL_RND_7 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=-0.5 -HV 2016 %s  | %FileCheck -check-prefix=DBL_RND_8 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=1.5 -HV 2016 %s  | %FileCheck -check-prefix=DBL_RND_9 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=-1.5 -HV 2016 %s  | %FileCheck -check-prefix=DBL_RND_10 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=1.6 -HV 2016  %s | %FileCheck -check-prefix=DBL_RND_11 %s
// RUN: %dxc -T vs_6_0 -E dr -DVAL=1.3 -HV 2016  %s | %FileCheck -check-prefix=DBL_RND_12 %s

// round intrinsic could exhibit different behaviour for constant and runtime evaluations.
// E.g., for round(0.5): constant evaluation results in 1 (away from zero rounding), 
// while runtime evaluation results in 0 (nearest even rounding).
// 
// For back compat, DXC still preserves the above behavior for language versions 2016 or below.
// However, for newer language versions, DXC now always use nearest even for round() intrinsic in all
// cases.


// FLT_RND_1: call void @dx.op.storeOutput{{.*}} float 0.000000e+00
// FLT_RND_2: call void @dx.op.storeOutput{{.*}} float -0.000000e+00
// FLT_RND_3: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// FLT_RND_4: call void @dx.op.storeOutput{{.*}} float -2.000000e+00
// FLT_RND_5: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// FLT_RND_6: call void @dx.op.storeOutput{{.*}} float 1.000000e+00

// FLT_RND_7: select i1 %{{.*}}, float 0.000000e+00, float 1.000000e+00
// FLT_RND_8: select i1 %{{.*}}, float -0.000000e+00, float -1.000000e+00
// FLT_RND_9: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// FLT_RND_10: call void @dx.op.storeOutput{{.*}} float -2.000000e+00
// FLT_RND_11: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// FLT_RND_12: call void @dx.op.storeOutput{{.*}} float 1.000000e+00

// DBL_RND_1: call void @dx.op.storeOutput{{.*}} float 0.000000e+00
// DBL_RND_2: call void @dx.op.storeOutput{{.*}} float -0.000000e+00
// DBL_RND_3: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// DBL_RND_4: call void @dx.op.storeOutput{{.*}} float -2.000000e+00
// DBL_RND_5: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// DBL_RND_6: call void @dx.op.storeOutput{{.*}} float 1.000000e+00

// DBL_RND_7: select i1 %{{.*}}, float 0.000000e+00, float 1.000000e+00
// DBL_RND_8: select i1 %{{.*}}, float -0.000000e+00, float -1.000000e+00
// DBL_RND_9: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// DBL_RND_10: call void @dx.op.storeOutput{{.*}} float -2.000000e+00
// DBL_RND_11: call void @dx.op.storeOutput{{.*}} float 2.000000e+00
// DBL_RND_12: call void @dx.op.storeOutput{{.*}} float 1.000000e+00

float fr(float f : INPUT) : OUTPUT {
  if (f == VAL)
    return round(f);
  else
    return round(VAL);
}

RWStructuredBuffer<double> buf;

float dr() : OUTPUT {
  double d = buf[0];
  if (d == VAL)
    return round(d);
  else
    return round(VAL);
}