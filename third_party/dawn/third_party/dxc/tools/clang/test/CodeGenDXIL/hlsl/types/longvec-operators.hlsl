// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=2 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=3 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=4 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=5 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=6 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=7 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=8 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=9 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=10 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=11 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=12 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=13 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=14 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=15 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=16 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=17 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=18 %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=128 %s | FileCheck %s --check-prefixes=CHECK,NODBL

// Less exhaustive testing for some other types.
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=int      -DNUM=2 -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint     -DNUM=5 -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=double   -DNUM=3 -DDBL %s | FileCheck %s --check-prefixes=CHECK,DBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint64_t -DNUM=9 -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float16_t -DNUM=17 -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=int16_t   -DNUM=177 -DINT -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG

// Test relevant operators on an assortment vector sizes and types with 6.9 native vectors.

// Just a trick to capture the needed type spellings since the DXC version of FileCheck can't do that explicitly.
// Uses non vector buffer to avoid interacting with that implementation.
// CHECK: %dx.types.ResRet.[[TY:[a-z0-9]*]] = type { [[TYPE:[a-z_0-9]*]]

RWStructuredBuffer< TYPE > buf;

export void assignments(inout vector<TYPE, NUM> things[10], TYPE scales[10]);
export vector<TYPE, NUM> arithmetic(inout vector<TYPE, NUM> things[11])[11];
export vector<TYPE, NUM> scarithmetic(inout vector<TYPE, NUM> things[10], TYPE scales[10])[10];
export vector<bool, NUM> logic(vector<bool, NUM> truth[10], vector<TYPE, NUM> consequences[10])[10];
export vector<TYPE, NUM> index(vector<TYPE, NUM> things[10], int i, TYPE val)[10];

struct Interface {
  vector<TYPE, NUM> assigned[10];
  vector<TYPE, NUM> arithmeticked[11];
  vector<TYPE, NUM> scarithmeticked[10];
  vector<bool, NUM> logicked[10];
  vector<TYPE, NUM> indexed[10];
  TYPE scales[10];
};

// A mixed-type overload to test overload resolution and mingle different vector element types in ops
// Test assignment operators.
// CHECK-LABEL: define void @"\01?assignments
export void assignments(inout vector<TYPE, NUM> things[10], TYPE scales[10]) {

  // Another trick to capture the size.
  // CHECK: [[res:%[0-9]*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle %{{[^,]*}}, i32 [[NUM:[0-9]*]]
  // CHECK: [[scl:%[0-9]*]] = extractvalue %dx.types.ResRet.[[TY]] [[res]], 0
  TYPE scalar = buf.Load(NUM);

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl]], i32 0
  // CHECK: [[res0:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res0]], <[[NUM]] x [[TYPE]]>* [[add0]]
  things[0] = scalar;

  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add5]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add1]]
  // CHECK: [[res1:%[0-9]*]] = [[ADD:f?add( fast)?]] <[[NUM]] x [[TYPE]]> [[vec1]], [[vec5]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[add1]]
  things[1] += things[5];

   // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add6]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[res2:%[0-9]*]] = [[SUB:f?sub( fast)?]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec6]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[add2]]
  things[2] -= things[6];

  // CHECK: [[add7:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 7
  // CHECK: [[vec7:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add7]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add3]]
  // CHECK: [[res3:%[0-9]*]] = [[MUL:f?mul( fast)?]] <[[NUM]] x [[TYPE]]> [[vec3]], [[vec7]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[add3]]
  things[3] *= things[7];

  // CHECK: [[add8:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 8
  // CHECK: [[vec8:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add8]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add4]]
  // CHECK: [[res4:%[0-9]*]] = [[DIV:[ufs]?div( fast)?]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec8]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[add4]]
  things[4] /= things[8];

  // CHECK: [[add9:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 9
  // CHECK: [[vec9:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add9]]
#ifdef DBL
  // DBL can't use remainder operator, do something anyway to keep the rest consistent.
  // DBL: [[fvec9:%[0-9]*]] = fptrunc <[[NUM]] x double> [[vec9]] to <[[NUM]] x float>
  // DBL: [[fvec5:%[0-9]*]] = fptrunc <[[NUM]] x double> [[vec5]] to <[[NUM]] x float>
  // DBL: [[fres5:%[0-9]*]] = [[REM:[ufs]?rem( fast)?]] <[[NUM]] x float> [[fvec5]], [[fvec9]]
  // DBL: [[res5:%[0-9]*]] = fpext <[[NUM]] x float> [[fres5]] to <[[NUM]] x double>
  vector<float,NUM> f9 = things[9];
  vector<float,NUM> f5 = things[5];
  f5 %= f9;
  things[5] = f5;
#else
  // NODBL: [[res5:%[0-9]*]] = [[REM:[ufs]?rem( fast)?]] <[[NUM]] x [[TYPE]]> [[vec5]], [[vec9]]
  things[5] %= things[9];
#endif
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[add5]]

  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 1
  // CHECK: [[scl1:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add1]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl1]], i32 0
  // CHECK: [[spt1:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res6:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[spt1]], [[vec6]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res6]], <[[NUM]] x [[TYPE]]>* [[add6]]
  things[6] += scales[1];

  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 2
  // CHECK: [[scl2:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add2]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl2]], i32 0
  // CHECK: [[spt2:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res7:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[vec7]], [[spt2]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res7]], <[[NUM]] x [[TYPE]]>* [[add7]]
  things[7] -= scales[2];

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 3
  // CHECK: [[scl3:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add3]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl3]], i32 0
  // CHECK: [[spt3:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res8:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[spt3]], [[vec8]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res8]], <[[NUM]] x [[TYPE]]>* [[add8]]
  things[8] *= scales[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 4
  // CHECK: [[scl4:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add4]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl4]], i32 0
  // CHECK: [[spt4:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res9:%[0-9]*]] = [[DIV]] <[[NUM]] x [[TYPE]]> [[vec9]], [[spt4]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res9]], <[[NUM]] x [[TYPE]]>* [[add9]]
  things[9] /= scales[4];

}

// Test arithmetic operators.
// CHECK-LABEL: define void @"\01?arithmetic
export vector<TYPE, NUM> arithmetic(inout vector<TYPE, NUM> things[11])[11] {
  vector<TYPE, NUM> res[11];
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: [[res1:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add0]]
  // CHECK: [[res0:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]>
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add1]]
  res[0] = -things[0];
  res[1] = +things[0];

  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[res2:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec1]]
  res[2] = things[1] + things[2];

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add3]]
  // CHECK: [[res3:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec3]]
  res[3] = things[2] - things[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add4]]
  // CHECK: [[res4:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec3]]
  res[4] = things[3] * things[4];

  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add5]]
  // CHECK: [[res5:%[0-9]*]] = [[DIV]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  res[5] = things[4] / things[5];

  // DBL: [[fvec5:%[0-9]*]] = fptrunc <[[NUM]] x double> [[vec5]] to <[[NUM]] x float>
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add6]]
#ifdef DBL
  // DBL can't use remainder operator, do something anyway to keep the rest consistent.
  // DBL: [[fvec6:%[0-9]*]] = fptrunc <[[NUM]] x double> [[vec6]] to <[[NUM]] x float>
  // DBL: [[fres6:%[0-9]*]] = [[REM]] <[[NUM]] x float> [[fvec5]], [[fvec6]]
  // DBL: [[res6:%[0-9]*]] = fpext <[[NUM]] x float> [[fres6]] to <[[NUM]] x double>
  res[6] = (vector<float,NUM>)things[5] % (vector<float,NUM>)things[6];
#else
  // NODBL: [[res6:%[0-9]*]] = [[REM]] <[[NUM]] x [[TYPE]]> [[vec5]], [[vec6]]
  res[6] = things[5] % things[6];
#endif

  // CHECK: [[add7:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 7
  // CHECK: [[vec7:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add7]]
  // CHECK: [[res7:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec7]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res7]], <[[NUM]] x [[TYPE]]>* [[add7]]
  res[7] = things[7]++;

  // CHECK: [[add8:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 8
  // CHECK: [[vec8:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add8]]
  // CHECK: [[res8:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec8]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res8]], <[[NUM]] x [[TYPE]]>* [[add8]]
  res[8] = things[8]--;

  // CHECK: [[add9:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 9
  // CHECK: [[vec9:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add9]]
  // CHECK: [[res9:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec9]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res9]], <[[NUM]] x [[TYPE]]>* [[add9]]
  res[9] = ++things[9];

  // CHECK: [[add10:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 10
  // CHECK: [[vec10:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add10]]
  // CHECK: [[res10:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec10]]
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res10]], <[[NUM]] x [[TYPE]]>* [[add10]]
  res[10] = --things[10];

  // Stores into res[]. Previous were for things[] inout.
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res0]], <[[NUM]] x [[TYPE]]>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 1
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 2
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 3
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 4
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 5
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 6
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res6]], <[[NUM]] x [[TYPE]]>* [[add6]]
  // These two were post ops, so the original value goes into res[].
  // CHECK: [[add7:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 7
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec7]], <[[NUM]] x [[TYPE]]>* [[add7]]
  // CHECK: [[add8:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 8
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec8]], <[[NUM]] x [[TYPE]]>* [[add8]]
  // CHECK: [[add9:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 9
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res9]], <[[NUM]] x [[TYPE]]>* [[add9]]
  // CHECK: [[add10:%[0-9]*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 10
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res10]], <[[NUM]] x [[TYPE]]>* [[add10]]
  // CHECK: ret void


  return res;
}

// Test arithmetic operators with scalars.
// CHECK-LABEL: define void @"\01?scarithmetic
export vector<TYPE, NUM> scarithmetic(inout vector<TYPE, NUM> things[10], TYPE scales[10])[10] {
  vector<TYPE, NUM> res[10];
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: [[vec0:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add6]]
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 0
  // CHECK: [[scl0:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add0]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl0]], i32 0
  // CHECK: [[spt0:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[spt0]], [[vec0]]
  res[0] = things[0] + scales[0];

  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 1
  // CHECK: [[scl1:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add1]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl1]], i32 0
  // CHECK: [[spt1:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res1:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[vec1]], [[spt1]]
  res[1] = things[1] - scales[1];


  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 2
  // CHECK: [[scl2:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add2]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl2]], i32 0
  // CHECK: [[spt2:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res2:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[spt2]], [[vec2]]
  res[2] = things[2] * scales[2];

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 3
  // CHECK: [[scl3:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add3]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl3]], i32 0
  // CHECK: [[spt3:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res3:%[0-9]*]] = [[DIV]] <[[NUM]] x [[TYPE]]> [[vec3]], [[spt3]]
  res[3] = things[3] / scales[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 4
  // CHECK: [[scl4:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add4]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl4]], i32 0
  // CHECK: [[spt4:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res4:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[spt4]], [[vec4]]
  res[4] = scales[4] + things[4];

  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 5
  // CHECK: [[scl5:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add5]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl5]], i32 0
  // CHECK: [[spt5:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res5:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[spt5]], [[vec5]]
  res[5] = scales[5] - things[5];

  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %scales, i32 0, i32 6
  // CHECK: [[scl6:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[add6]]
  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl6]], i32 0
  // CHECK: [[spt6:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res6:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[spt6]], [[vec6]]
  res[6] = scales[6] * things[6];

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res0]], <[[NUM]] x [[TYPE]]>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 1
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 2
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 3
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 4
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 5
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %agg.result, i32 0, i32 6
  // CHECK: store <[[NUM]] x [[TYPE]]> [[res6]], <[[NUM]] x [[TYPE]]>* [[add6]]
  // CHECK: ret void


  return res;
}

// Test logic operators.
// Only permissable in pre-HLSL2021
// CHECK-LABEL: define void @"\01?logic
export vector<bool, NUM> logic(vector<bool, NUM> truth[10], vector<TYPE, NUM> consequences[10])[10] {
  vector<bool, NUM> res[10];
  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 0
  // CHECK: [[vec0:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add0]]
  // CHECK: [[cmp:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec0]], zeroinitializer
  // CHECK: [[cmp0:%[0-9]*]] = icmp eq <[[NUM]] x i1> [[cmp]], zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp0]] to <[[NUM]] x i32>
  res[0] = !truth[0];

  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add1]]
  // CHECK: [[bvec1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec1]], zeroinitializer
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add2]]
  // CHECK: [[bvec2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec2]], zeroinitializer
  // CHECK: [[bres1:%[0-9]*]] = or <[[NUM]] x i1> [[bvec2]], [[bvec1]]
  // CHECK: [[res1:%[0-9]*]] = zext <[[NUM]] x i1> [[bres1]] to <[[NUM]] x i32>
  res[1] = truth[1] || truth[2];

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add3]]
  // CHECK: [[bvec3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec3]], zeroinitializer
  // CHECK: [[bres2:%[0-9]*]] = and <[[NUM]] x i1> [[bvec3]], [[bvec2]]
  // CHECK: [[res2:%[0-9]*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  res[2] = truth[2] && truth[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add4]]
  // CHECK: [[bvec4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec4]], zeroinitializer
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %truth, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x i32>, <[[NUM]] x i32>* [[add5]]
  // CHECK: [[bvec5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[vec5]], zeroinitializer
  // CHECK: [[bres3:%[0-9]*]] = select <[[NUM]] x i1> [[bvec3]], <[[NUM]] x i1> [[bvec4]], <[[NUM]] x i1> [[bvec5]]
  // CHECK: [[res3:%[0-9]*]] = zext <[[NUM]] x i1> [[bres3]] to <[[NUM]] x i32>
  res[3] = truth[3] ? truth[4] : truth[5];

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 0
  // CHECK: [[vec0:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 1
  // CHECK: [[vec1:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add1]]
  // CHECK: [[cmp4:%[0-9]*]] = [[CMP:[fi]?cmp( fast)?]] {{o?}}eq <[[NUM]] x [[TYPE]]> [[vec0]], [[vec1]]
  // CHECK: [[res4:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp4]] to <[[NUM]] x i32>
  res[4] = consequences[0] == consequences[1];

  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 2
  // CHECK: [[vec2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[cmp5:%[0-9]*]] = [[CMP]] {{u?}}ne <[[NUM]] x [[TYPE]]> [[vec1]], [[vec2]]
  // CHECK: [[res5:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp5]] to <[[NUM]] x i32>
  res[5] = consequences[1] != consequences[2];

  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 3
  // CHECK: [[vec3:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add3]]
  // CHECK: [[cmp6:%[0-9]*]] = [[CMP]] {{[osu]?}}lt <[[NUM]] x [[TYPE]]> [[vec2]], [[vec3]]
  // CHECK: [[res6:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp6]] to <[[NUM]] x i32>
  res[6] = consequences[2] <  consequences[3];

  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 4
  // CHECK: [[vec4:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add4]]
  // CHECK: [[cmp7:%[0-9]*]] = [[CMP]] {{[osu]]?}}gt <[[NUM]] x [[TYPE]]> [[vec3]], [[vec4]]
  // CHECK: [[res7:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp7]] to <[[NUM]] x i32>
  res[7] = consequences[3] >  consequences[4];

  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 5
  // CHECK: [[vec5:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add5]]
  // CHECK: [[cmp8:%[0-9]*]] = [[CMP]] {{[osu]]?}}le <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  // CHECK: [[res8:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp8]] to <[[NUM]] x i32>
  res[8] = consequences[4] <= consequences[5];

  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %consequences, i32 0, i32 6
  // CHECK: [[vec6:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add6]]
  // CHECK: [[cmp9:%[0-9]*]] = [[CMP]] {{[osu]?}}ge <[[NUM]] x [[TYPE]]> [[vec5]], [[vec6]]
  // CHECK: [[res9:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp9]] to <[[NUM]] x i32>
  res[9] = consequences[5] >= consequences[6];

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 0
  // CHECK: store <[[NUM]] x i32> [[res0]], <[[NUM]] x i32>* [[add0]]
  // CHECK: [[add1:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 1
  // CHECK: store <[[NUM]] x i32> [[res1]], <[[NUM]] x i32>* [[add1]]
  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 2
  // CHECK: store <[[NUM]] x i32> [[res2]], <[[NUM]] x i32>* [[add2]]
  // CHECK: [[add3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 3
  // CHECK: store <[[NUM]] x i32> [[res3]], <[[NUM]] x i32>* [[add3]]
  // CHECK: [[add4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 4
  // CHECK: store <[[NUM]] x i32> [[res4]], <[[NUM]] x i32>* [[add4]]
  // CHECK: [[add5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 5
  // CHECK: store <[[NUM]] x i32> [[res5]], <[[NUM]] x i32>* [[add5]]
  // CHECK: [[add6:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 6
  // CHECK: store <[[NUM]] x i32> [[res6]], <[[NUM]] x i32>* [[add6]]
  // CHECK: [[add7:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 7
  // CHECK: store <[[NUM]] x i32> [[res7]], <[[NUM]] x i32>* [[add7]]
  // CHECK: [[add8:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 8
  // CHECK: store <[[NUM]] x i32> [[res8]], <[[NUM]] x i32>* [[add8]]
  // CHECK: [[add9:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x i32>], [10 x <[[NUM]] x i32>]* %agg.result, i32 0, i32 9
  // CHECK: store <[[NUM]] x i32> [[res9]], <[[NUM]] x i32>* [[add9]]
  // CHECK: ret void

  return res;
}

static const int Ix = 2;

// Test indexing operators
// CHECK-LABEL: define void @"\01?index
export vector<TYPE, NUM> index(vector<TYPE, NUM> things[10], int i, TYPE val)[10] {
  vector<TYPE, NUM> res[10];

  // CHECK: [[res:%[0-9]*]] = alloca [10 x <[[NUM]] x [[TYPE]]>]
  // CHECK: [[res0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> zeroinitializer, <[[NUM]] x [[TYPE]]>* [[res0]]
  res[0] = 0;

  // CHECK: [[resi:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 %i
  // CHECK: store <[[NUM]] x [[TYPE]]> <[[TYPE]] {{(1|0xH3C00).*}}>, <[[NUM]] x [[TYPE]]>* [[resi]]
  res[i] = 1;

  // CHECK: [[res2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 2
  // CHECK: store <[[NUM]] x [[TYPE]]> <[[TYPE]] {{(2|0xH4000).*}}>, <[[NUM]] x [[TYPE]]>* [[res2]]
  res[Ix] = 2;

  // CHECK: [[add0:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // CHECK: [[thg0:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add0]]
  // CHECK: [[res3:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 3
  // CHECK: store <[[NUM]] x [[TYPE]]> [[thg0]], <[[NUM]] x [[TYPE]]>* [[res3]]
  res[3] = things[0];

  // CHECK: [[addi:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 %i
  // CHECK: [[thgi:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[addi]]
  // CHECK: [[res4:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 4
  // CHECK: store <[[NUM]] x [[TYPE]]> [[thgi]], <[[NUM]] x [[TYPE]]>* [[res4]]
  res[4] = things[i];

  // CHECK: [[add2:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // CHECK: [[thg2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[add2]]
  // CHECK: [[res5:%[0-9]*]] = getelementptr inbounds [10 x <[[NUM]] x [[TYPE]]>], [10 x <[[NUM]] x [[TYPE]]>]* [[res]], i32 0, i32 5
  // CHECK: store <[[NUM]] x [[TYPE]]> [[thg2]], <[[NUM]] x [[TYPE]]>* [[res5]]
  res[5] = things[Ix];
  // CHECK: ret void
  return res;
}

#ifdef INT
// Test bit twiddling operators.
// INT-LABEL: define void @"\01?bittwiddlers
export void bittwiddlers(inout vector<TYPE, NUM> things[13]) {
  // INT: [[adr1:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 1
  // INT: [[ld1:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr1]]
  // INT: [[res1:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[ld1]], <[[TYPE]] -1
  // INT: [[adr0:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 0
  // INT: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[adr0]]
  things[0] = ~things[1];

  // INT: [[adr2:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 2
  // INT: [[ld2:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr2]]
  // INT: [[adr3:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 3
  // INT: [[ld3:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr3]]
  // INT: [[res1:%[0-9]*]] = or <[[NUM]] x [[TYPE]]> [[ld3]], [[ld2]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res1]], <[[NUM]] x [[TYPE]]>* [[adr1]]
  things[1] = things[2] | things[3];

  // INT: [[adr4:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 4
  // INT: [[ld4:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr4]]
  // INT: [[res2:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[ld4]], [[ld3]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res2]], <[[NUM]] x [[TYPE]]>* [[adr2]]
  things[2] = things[3] & things[4];

  // INT: [[adr5:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 5
  // INT: [[ld5:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr5]]
  // INT: [[res3:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[ld4]], [[ld5]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res3]], <[[NUM]] x [[TYPE]]>* [[adr3]]
  things[3] = things[4] ^ things[5];

  // INT: [[adr6:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 6
  // INT: [[ld6:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr6]]
  // INT: [[shv6:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[ld6]]
  // INT: [[res4:%[0-9]*]] = shl <[[NUM]] x [[TYPE]]> [[ld5]], [[shv6]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res4]], <[[NUM]] x [[TYPE]]>* [[adr4]]
  things[4] = things[5] << things[6];

  // INT: [[adr7:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 7
  // INT: [[ld7:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr7]]
  // INT: [[shv7:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[ld7]]
  // UNSIG: [[res5:%[0-9]*]] = lshr <[[NUM]] x [[TYPE]]> [[ld6]], [[shv7]]
  // SIG: [[res5:%[0-9]*]] = ashr <[[NUM]] x [[TYPE]]> [[ld6]], [[shv7]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res5]], <[[NUM]] x [[TYPE]]>* [[adr5]]
  things[5] = things[6] >> things[7];

  // INT: [[adr8:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 8
  // INT: [[ld8:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr8]]
  // INT: [[res6:%[0-9]*]] = or <[[NUM]] x [[TYPE]]> [[ld8]], [[ld6]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res6]], <[[NUM]] x [[TYPE]]>* [[adr6]]
  things[6] |= things[8];

  // INT: [[adr9:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 9
  // INT: [[ld9:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr9]]
  // INT: [[res7:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[ld9]], [[ld7]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res7]], <[[NUM]] x [[TYPE]]>* [[adr7]]
  things[7] &= things[9];

  // INT: [[adr10:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 10
  // INT: [[ld10:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr10]]
  // INT: [[res8:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[ld8]], [[ld10]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res8]], <[[NUM]] x [[TYPE]]>* [[adr8]]
  things[8] ^= things[10];

  // INT: [[adr11:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 11
  // INT: [[ld11:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr11]]
  // INT: [[shv11:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[ld11]]
  // INT: [[res9:%[0-9]*]] = shl <[[NUM]] x [[TYPE]]> [[ld9]], [[shv11]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res9]], <[[NUM]] x [[TYPE]]>* [[adr9]]
  things[9] <<= things[11];

  // INT: [[adr12:%[0-9]*]] = getelementptr inbounds [13 x <[[NUM]] x [[TYPE]]>], [13 x <[[NUM]] x [[TYPE]]>]* %things, i32 0, i32 12
  // INT: [[ld12:%[0-9]*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr12]]
  // INT: [[shv12:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[ld12]]
  // UNSIG: [[res10:%[0-9]*]] = lshr <[[NUM]] x [[TYPE]]> [[ld10]], [[shv12]]
  // SIG: [[res10:%[0-9]*]] = ashr <[[NUM]] x [[TYPE]]> [[ld10]], [[shv12]]
  // INT: store <[[NUM]] x [[TYPE]]> [[res10]], <[[NUM]] x [[TYPE]]>* [[adr10]]
  things[10] >>= things[12];

  // INT: ret void
}
#endif // INT
