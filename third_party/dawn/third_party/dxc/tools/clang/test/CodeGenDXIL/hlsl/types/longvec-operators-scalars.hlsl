// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float  %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=int       %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint      %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=double    -DDBL %s | FileCheck %s --check-prefixes=CHECK,DBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=int64_t   %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint64_t  %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float16_t -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=int16_t   -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint16_t  -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL

// Test relevant operators on an assortment bool vector sizes and types with 6.9 native vectors.

// Just a trick to capture the needed type spellings since the DXC version of FileCheck can't do that explicitly.
// CHECK: %dx.types.ResRet.[[TY:[a-z0-9]*]] = type { [[TYPE:[a-z0-9_]*]]
RWStructuredBuffer<TYPE> buf;

export void assignments(inout TYPE things[10], TYPE scales[10]);
export TYPE arithmetic(inout TYPE things[11])[11];
export bool logic(bool truth[10], TYPE consequences[10])[10];
export TYPE index(TYPE things[10], int i, TYPE val)[10];

struct Interface {
  TYPE assigned[10];
  TYPE arithmeticked[11];
  bool logicked[10];
  TYPE indexed[10];
  TYPE scales[10];
};

#if 0
// Requires vector loading support. Enable when available.
RWStructuredBuffer<Interface> Input;
RWStructuredBuffer<Interface> Output;

TYPE g_val;

[shader("compute")]
[numthreads(8,1,1)]
void main(uint GI : SV_GroupIndex) {
  assignments(Output[GI].assigned, Input[GI].scales);
  Output[GI].arithmeticked = arithmetic(Input[GI].arithmeticked);
  Output[GI].logicked = logic(Input[GI].logicked, Input[GI].assigned);
  Output[GI].indexed = index(Input[GI].indexed, GI, g_val);
}
#endif

// A mixed-type overload to test overload resolution and mingle different vector element types in ops
// Test assignment operators.
// CHECK-LABEL: define void @"\01?assignments
export void assignments(inout TYPE things[10]) {

  // CHECK: [[buf:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle {{%.*}}, i32 1, i32 0, i8 1, i32 {{(8|4|2)}})
  // CHECK: [[res0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[buf]], 0
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 0
  // CHECK: store [[TYPE]] [[res0]], [[TYPE]]* [[adr0]]
  things[0] = buf.Load(1);

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 5
  // CHECK: [[val5:%.*]] = load [[TYPE]], [[TYPE]]* [[adr5]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 1
  // CHECK: [[val1:%.*]] = load [[TYPE]], [[TYPE]]* [[adr1]]
  // CHECK: [[res1:%.*]] = [[ADD:f?add( fast| nsw)?]] [[TYPE]] [[val1]], [[val5]]
  // CHECK: store [[TYPE]] [[res1]], [[TYPE]]* [[adr1]]
  things[1] += things[5];

  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 6
  // CHECK: [[val6:%.*]] = load [[TYPE]], [[TYPE]]* [[adr6]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 2
  // CHECK: [[val2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[res2:%.*]] = [[SUB:f?sub( fast| nsw)?]] [[TYPE]] [[val2]], [[val6]]
  // CHECK: store [[TYPE]] [[res2]], [[TYPE]]* [[adr2]]
  things[2] -= things[6];

  // CHECK: [[adr7:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 7
  // CHECK: [[val7:%.*]] = load [[TYPE]], [[TYPE]]* [[adr7]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 3
  // CHECK: [[val3:%.*]] = load [[TYPE]], [[TYPE]]* [[adr3]]
  // CHECK: [[res3:%.*]] = [[MUL:f?mul( fast| nsw)?]] [[TYPE]] [[val3]], [[val7]]
  // CHECK: store [[TYPE]] [[res3]], [[TYPE]]* [[adr3]]
  things[3] *= things[7];

  // CHECK: [[adr8:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 8
  // CHECK: [[val8:%.*]] = load [[TYPE]], [[TYPE]]* [[adr8]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 4
  // CHECK: [[val4:%.*]] = load [[TYPE]], [[TYPE]]* [[adr4]]
  // CHECK: [[res4:%.*]] = [[DIV:[ufs]?div( fast| nsw)?]] [[TYPE]] [[val4]], [[val8]]
  // CHECK: store [[TYPE]] [[res4]], [[TYPE]]* [[adr4]]
  things[4] /= things[8];

  // CHECK: [[adr9:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 9
  // CHECK: [[val9:%.*]] = load [[TYPE]], [[TYPE]]* [[adr9]]
#ifdef DBL
  // DBL: [[fvec9:%.*]] = fptrunc double [[val9]] to float
  // DBL: [[fvec5:%.*]] = fptrunc double [[val5]] to float
  // DBL: [[fres5:%.*]] = [[REM:[ufs]?rem( fast| nsw)?]] float [[fvec5]], [[fvec9]]
  // DBL: [[res5:%.*]] = fpext float [[fres5]] to double
  float f9 = things[9];
  float f5 = things[5];
  f5 %= f9;
  things[5] = f5;
#else
  // NODBL: [[res5:%.*]] = [[REM:[ufs]?rem( fast| nsw)?]] [[TYPE]] [[val5]], [[val9]]
  things[5] %= things[9];
#endif
  // CHECK: store [[TYPE]] [[res5]], [[TYPE]]* [[adr5]]
}

// Test arithmetic operators.
// CHECK-LABEL: define void @"\01?arithmetic
export TYPE arithmetic(inout TYPE things[11])[11] {
  TYPE res[11];
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 0
  // CHECK: [[res0:%.*]] = load [[TYPE]], [[TYPE]]* [[adr0]]
  // CHECK: [[res1:%.*]] = [[SUB]] [[TYPE]] {{-?(0|0\.0*e\+0*|0xH8000)}}, [[res0]]
  res[0] = +things[0];
  res[1] = -things[0];

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 1
  // CHECK: [[val1:%.*]] = load [[TYPE]], [[TYPE]]* [[adr1]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 2
  // CHECK: [[val2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[res2:%.*]] = [[ADD]] [[TYPE]] [[val2]], [[val1]]
  res[2] = things[1] + things[2];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 3
  // CHECK: [[val3:%.*]] = load [[TYPE]], [[TYPE]]* [[adr3]]
  // CHECK: [[res3:%.*]] = [[SUB]] [[TYPE]] [[val2]], [[val3]]
  res[3] = things[2] - things[3];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 4
  // CHECK: [[val4:%.*]] = load [[TYPE]], [[TYPE]]* [[adr4]]
  // CHECK: [[res4:%.*]] = [[MUL]] [[TYPE]] [[val4]], [[val3]]
  res[4] = things[3] * things[4];

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 5
  // CHECK: [[val5:%.*]] = load [[TYPE]], [[TYPE]]* [[adr5]]
  // CHECK: [[res5:%.*]] = [[DIV]] [[TYPE]] [[val4]], [[val5]]
  res[5] = things[4] / things[5];

  // DBL: [[fvec5:%.*]] = fptrunc double [[val5]] to float
  // CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 6
  // CHECK: [[val6:%.*]] = load [[TYPE]], [[TYPE]]* [[adr6]]
#ifdef DBL
  // DBL: [[fvec6:%.*]] = fptrunc double [[val6]] to float
  // DBL: [[fres6:%.*]] = [[REM]] float [[fvec5]], [[fvec6]]
  // DBL: [[res6:%.*]] = fpext float [[fres6]] to double
  res[6] = (float)things[5] % (float)things[6];
#else
  // NODBL: [[res6:%.*]] = [[REM]] [[TYPE]] [[val5]], [[val6]]
  res[6] = things[5] % things[6];
#endif

  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 7
  // CHECK: [[val7:%.*]] = load [[TYPE]], [[TYPE]]* [[adr7]]
  // CHECK: [[res7:%.*]] = [[ADD:f?add( fast| nsw)?]] [[TYPE]] [[val7]], {{(1|1\.?0*e?\+?0*|0xH3C00)}}
  // CHECK: store [[TYPE]] [[res7]], [[TYPE]]* [[adr7]]
  res[7] = things[7]++;

  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 8
  // CHECK: [[val8:%.*]] = load [[TYPE]], [[TYPE]]* [[adr8]]
  // CHECK: [[res8:%.*]] = [[ADD]] [[TYPE]] [[val8]]
  // CHECK: store [[TYPE]] [[res8]], [[TYPE]]* [[adr8]]
  res[8] = things[8]--;

  // CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 9
  // CHECK: [[val9:%.*]] = load [[TYPE]], [[TYPE]]* [[adr9]]
  // CHECK: [[res9:%.*]] = [[ADD]] [[TYPE]] [[val9]]
  // CHECK: store [[TYPE]] [[res9]], [[TYPE]]* [[adr9]]
  res[9] = ++things[9];

  // CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 10
  // CHECK: [[val10:%.*]] = load [[TYPE]], [[TYPE]]* [[adr10]]
  // CHECK: [[res10:%.*]] = [[ADD]] [[TYPE]] [[val10]]
  // CHECK: store [[TYPE]] [[res10]], [[TYPE]]* [[adr10]]
  res[10] = --things[10];

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 0
  // CHECK: store [[TYPE]] [[res0]], [[TYPE]]* [[adr0]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 1
  // CHECK: store [[TYPE]] [[res1]], [[TYPE]]* [[adr1]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 2
  // CHECK: store [[TYPE]] [[res2]], [[TYPE]]* [[adr2]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 3
  // CHECK: store [[TYPE]] [[res3]], [[TYPE]]* [[adr3]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 4
  // CHECK: store [[TYPE]] [[res4]], [[TYPE]]* [[adr4]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 5
  // CHECK: store [[TYPE]] [[res5]], [[TYPE]]* [[adr5]]
  // CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 6
  // CHECK: store [[TYPE]] [[res6]], [[TYPE]]* [[adr6]]
  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 7
  // This is a post op, so the original value goes into res[].
  // CHECK: store [[TYPE]] [[val7]], [[TYPE]]* [[adr7]]
  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 8
  // This is a post op, so the original value goes into res[].
  // CHECK: store [[TYPE]] [[val8]], [[TYPE]]* [[adr8]]
  // CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 9
  // CHECK: store [[TYPE]] [[res9]], [[TYPE]]* [[adr9]]
  // CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 10
  // CHECK: store [[TYPE]] [[res10]], [[TYPE]]* [[adr10]]
  // CHECK: ret void
  return res;
}

// Test logic operators.
// Only permissable in pre-HLSL2021
// CHECK-LABEL: define void @"\01?logic
export bool logic(bool truth[10], TYPE consequences[10])[10] {
  bool res[10];
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 0
  // CHECK: [[val0:%.*]] = load i32, i32* [[adr0]]
  // CHECK: [[res0:%.*]] = xor i32 [[val0]], 1
  res[0] = !truth[0];

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 1
  // CHECK: [[val1:%.*]] = load i32, i32* [[adr1]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 2
  // CHECK: [[val2:%.*]] = load i32, i32* [[adr2]]
  // CHECK: [[res1:%.*]] = or i32 [[val2]], [[val1]]
  res[1] = truth[1] || truth[2];

  // CHECK: [[bvec2:%.*]] = icmp ne i32 [[val2]], 0
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 3
  // CHECK: [[val3:%.*]] = load i32, i32* [[adr3]]
  // CHECK: [[bvec3:%.*]] = icmp ne i32 [[val3]], 0
  // CHECK: [[bres2:%.*]] = and i1 [[bvec2]], [[bvec3]]
  // CHECK: [[res2:%.*]] = zext i1 [[bres2]] to i32
  res[2] = truth[2] && truth[3];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 4
  // CHECK: [[val4:%.*]] = load i32, i32* [[adr4]]
  // CHECK: [[bvec4:%.*]] = icmp ne i32 [[val4]], 0
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 5
  // CHECK: [[val5:%.*]] = load i32, i32* [[adr5]]
  // CHECK: [[bvec5:%.*]] = icmp ne i32 [[val5]], 0
  // CHECK: [[bres3:%.*]] = select i1 [[bvec3]], i1 [[bvec4]], i1 [[bvec5]]
  // CHECK: [[res3:%.*]] = zext i1 [[bres3]] to i32
  res[3] = truth[3] ? truth[4] : truth[5];

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 0
  // CHECK: [[val0:%.*]] = load [[TYPE]], [[TYPE]]* [[adr0]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 1
  // CHECK: [[val1:%.*]] = load [[TYPE]], [[TYPE]]* [[adr1]]
  // CHECK: [[cmp4:%.*]] = [[CMP:[fi]?cmp( fast| nsw)?]] {{o?}}eq [[TYPE]] [[val0]], [[val1]]
  // CHECK: [[res4:%.*]] = zext i1 [[cmp4]] to i32
  res[4] = consequences[0] == consequences[1];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 2
  // CHECK: [[val2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[cmp5:%.*]] = [[CMP]] {{u?}}ne [[TYPE]] [[val1]], [[val2]]
  // CHECK: [[res5:%.*]] = zext i1 [[cmp5]] to i32
  res[5] = consequences[1] != consequences[2];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 3
  // CHECK: [[val3:%.*]] = load [[TYPE]], [[TYPE]]* [[adr3]]
  // CHECK: [[cmp6:%.*]] = [[CMP]] {{[osu]?}}lt [[TYPE]] [[val2]], [[val3]]
  // CHECK: [[res6:%.*]] = zext i1 [[cmp6]] to i32
  res[6] = consequences[2] <  consequences[3];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 4
  // CHECK: [[val4:%.*]] = load [[TYPE]], [[TYPE]]* [[adr4]]
  // CHECK: [[cmp7:%.*]] = [[CMP]] {{[osu]]?}}gt [[TYPE]] [[val3]], [[val4]]
  // CHECK: [[res7:%.*]] = zext i1 [[cmp7]] to i32
  res[7] = consequences[3] >  consequences[4];

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 5
  // CHECK: [[val5:%.*]] = load [[TYPE]], [[TYPE]]* [[adr5]]
  // CHECK: [[cmp8:%.*]] = [[CMP]] {{[osu]]?}}le [[TYPE]] [[val4]], [[val5]]
  // CHECK: [[res8:%.*]] = zext i1 [[cmp8]] to i32
  res[8] = consequences[4] <= consequences[5];

  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 6
  // CHECK: [[val6:%.*]] = load [[TYPE]], [[TYPE]]* [[adr6]]
  // CHECK: [[cmp9:%.*]] = [[CMP]] {{[osu]?}}ge [[TYPE]] [[val5]], [[val6]]
  // CHECK: [[res9:%.*]] = zext i1 [[cmp9]] to i32
  res[9] = consequences[5] >= consequences[6];

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 0
  // CHECK: store i32 [[res0]], i32* [[adr0]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 1
  // CHECK: store i32 [[res1]], i32* [[adr1]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 2
  // CHECK: store i32 [[res2]], i32* [[adr2]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 3
  // CHECK: store i32 [[res3]], i32* [[adr3]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 4
  // CHECK: store i32 [[res4]], i32* [[adr4]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 5
  // CHECK: store i32 [[res5]], i32* [[adr5]]
  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 6
  // CHECK: store i32 [[res6]], i32* [[adr6]]
  // CHECK: [[adr7:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 7
  // CHECK: store i32 [[res7]], i32* [[adr7]]
  // CHECK: [[adr8:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 8
  // CHECK: store i32 [[res8]], i32* [[adr8]]
  // CHECK: [[adr9:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 9
  // CHECK: store i32 [[res9]], i32* [[adr9]]

  // CHECK: ret void
  return res;
}

static const int Ix = 2;

// Test indexing operators
// CHECK-LABEL: define void @"\01?index
export TYPE index(TYPE things[10], int i)[10] {
  // CHECK: [[res:%.*]] = alloca [10 x [[TYPE]]]
  TYPE res[10];

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* [[res]], i32 0, i32 0
  // CHECK: store [[TYPE]] {{(0|0*\.?0*e?\+?0*|0xH0000)}}, [[TYPE]]* [[adr0]]
  res[0] = 0;

  // CHECK: [[adri:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* [[res]], i32 0, i32 %i
  // CHECK: store [[TYPE]] {{(1|1\.?0*e?\+?0*|0xH3C00)}}, [[TYPE]]* [[adri]]
  res[i] = 1;

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* [[res]], i32 0, i32 2
  // CHECK: store [[TYPE]] {{(2|2\.?0*e?\+?0*|0xH4000)}}, [[TYPE]]* [[adr2]]
  res[Ix] = 2;

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 0
  // CHECK: [[thg0:%.*]] = load [[TYPE]], [[TYPE]]* [[adr0]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* [[res]], i32 0, i32 3
  // CHECK: store [[TYPE]] [[thg0]], [[TYPE]]* [[adr3]]
  res[3] = things[0];

  // CHECK: [[adri:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 %i
  // CHECK: [[thgi:%.*]] = load [[TYPE]], [[TYPE]]* [[adri]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* [[res]], i32 0, i32 4
  // CHECK: store [[TYPE]] [[thgi]], [[TYPE]]* [[adr4]]
  res[4] = things[i];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 2
  // CHECK: [[thg2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* [[res]], i32 0, i32 5
  // CHECK: store [[TYPE]] [[thg2]], [[TYPE]]* [[adr5]]
  res[5] = things[Ix];
  // CHECK: ret void
  return res;
}
