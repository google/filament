// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float          %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=int      -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=double   -DDBL %s | FileCheck %s --check-prefixes=CHECK
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint64_t -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float16_t      -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=int16_t  -DINT -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG

// Test relevant operators on vec1s in 6.9 to ensure they continue to be treated as scalars.

#define VTYPE vector<TYPE, 1>

// Just a trick to capture the needed type spellings since the DXC version of FileCheck can't do that explicitly.
// CHECK: %dx.types.ResRet.[[TY:[a-z0-9]*]] = type { [[ELTY:[a-z0-9_]*]]
// CHECK: %"class.RWStructuredBuffer<{{.*}}>" = type { [[TYPE:.*]] }
RWStructuredBuffer<VTYPE> buf;

// A mixed-type overload to test overload resolution and mingle different vector element types in ops
// Test assignment operators.
// CHECK-LABEL: define void @"\01?assignments
export void assignments(inout VTYPE things[10]) {

  // CHECK: [[buf:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle {{%.*}}, i32 1, i32 0, i8 1, i32 {{8|4|2}})
  // CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[buf]], 0
  // CHECK: [[res0:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[val0]], i64 0
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 0
  // CHECK: store [[TYPE]] [[res0]], [[TYPE]]* [[adr0]]
  things[0] = buf.Load(1);

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 5
  // CHECK: [[ld5:%.*]] = load [[TYPE]], [[TYPE]]* [[adr5]]
  // CHECK: [[val5:%.*]] = extractelement [[TYPE]] [[ld5]], i32 0
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 1
  // CHECK: [[ld1:%.*]] = load [[TYPE]], [[TYPE]]* [[adr1]]
  // CHECK: [[val1:%.*]] = extractelement [[TYPE]] [[ld1]], i32 0
  // CHECK: [[add1:%.*]] = [[ADD:f?add( fast)?]] [[ELTY]] [[val1]], [[val5]]
  // CHECK: [[res1:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add1]], i32 0
  // CHECK: store [[TYPE]] [[res1]], [[TYPE]]* [[adr1]]
  things[1] += things[5];

  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 6
  // CHECK: [[ld6:%.*]] = load [[TYPE]], [[TYPE]]* [[adr6]]
  // CHECK: [[val6:%.*]] = extractelement [[TYPE]] [[ld6]], i32 0
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 2
  // CHECK: [[ld2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[val2:%.*]] = extractelement [[TYPE]] [[ld2]], i32 0
  // CHECK: [[sub2:%.*]] = [[SUB:f?sub( fast)?]] [[ELTY]] [[val2]], [[val6]]
  // CHECK: [[res2:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[sub2]], i32 0
  // CHECK: store [[TYPE]] [[res2]], [[TYPE]]* [[adr2]]
  things[2] -= things[6];

  // CHECK: [[adr7:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 7
  // CHECK: [[ld7:%.*]] = load [[TYPE]], [[TYPE]]* [[adr7]]
  // CHECK: [[val7:%.*]] = extractelement [[TYPE]] [[ld7]], i32 0
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 3
  // CHECK: [[ld3:%.*]] = load [[TYPE]], [[TYPE]]* [[adr3]]
  // CHECK: [[val3:%.*]] = extractelement [[TYPE]] [[ld3]], i32 0
  // CHECK: [[mul3:%.*]] = [[MUL:f?mul( fast)?]] [[ELTY]] [[val3]], [[val7]]
  // CHECK: [[res3:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[mul3]], i32 0
  // CHECK: store [[TYPE]] [[res3]], [[TYPE]]* [[adr3]]
  things[3] *= things[7];

  // CHECK: [[adr8:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 8
  // CHECK: [[ld8:%.*]] = load [[TYPE]], [[TYPE]]* [[adr8]]
  // CHECK: [[val8:%.*]] = extractelement [[TYPE]] [[ld8]], i32 0
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 4
  // CHECK: [[ld4:%.*]] = load [[TYPE]], [[TYPE]]* [[adr4]]
  // CHECK: [[val4:%.*]] = extractelement [[TYPE]] [[ld4]], i32 0
  // CHECK: [[div4:%.*]] = [[DIV:[ufs]?div( fast)?]] [[ELTY]] [[val4]], [[val8]]
  // CHECK: [[res4:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[div4]], i32 0
  // CHECK: store [[TYPE]] [[res4]], [[TYPE]]* [[adr4]]
  things[4] /= things[8];

#ifndef DBL
  // NODBL: [[adr9:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 9
  // NODBL: [[ld9:%.*]] = load [[TYPE]], [[TYPE]]* [[adr9]]
  // NODBL: [[val9:%.*]] = extractelement [[TYPE]] [[ld9]]
  // NODBL: [[rem5:%.*]] = [[REM:[ufs]?rem( fast)?]] [[ELTY]] [[val5]], [[val9]]
  // NODBL: [[res5:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[rem5]], i32 0
  // NODBL: store [[TYPE]] [[res5]], [[TYPE]]* [[adr5]]
  things[5] %= things[9];
#endif
}

// Test arithmetic operators.
// CHECK-LABEL: define void @"\01?arithmetic
export VTYPE arithmetic(inout VTYPE things[11])[11] {
  VTYPE res[11];
  // CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 0
  // CHECK: [[res0:%.*]] = load [[TYPE]], [[TYPE]]* [[adr0]]
  // CHECK: [[val0:%.*]] = extractelement [[TYPE]] [[res0]], i32 0
  // CHECK: [[sub1:%.*]] = [[SUB]] [[ELTY]] {{-?(0|0\.?0*e?\+?0*|0xH8000)}}, [[val0]]
  res[0] = +things[0];
  res[1] = -things[0];

  // CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 1
  // CHECK: [[ld1:%.*]] = load [[TYPE]], [[TYPE]]* [[adr1]]
  // CHECK: [[val1:%.*]] = extractelement [[TYPE]] [[ld1]], i32 0
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 2
  // CHECK: [[ld2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[val2:%.*]] = extractelement [[TYPE]] [[ld2]], i32 0
  // CHECK: [[add2:%.*]] = [[ADD]] [[ELTY]] [[val2]], [[val1]]
  res[2] = things[1] + things[2];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 3
  // CHECK: [[ld3:%.*]] = load [[TYPE]], [[TYPE]]* [[adr3]]
  // CHECK: [[val3:%.*]] = extractelement [[TYPE]] [[ld3]], i32 0
  // CHECK: [[sub3:%.*]] = [[SUB]] [[ELTY]] [[val2]], [[val3]]
  res[3] = things[2] - things[3];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 4
  // CHECK: [[ld4:%.*]] = load [[TYPE]], [[TYPE]]* [[adr4]]
  // CHECK: [[val4:%.*]] = extractelement [[TYPE]] [[ld4]], i32 0
  // CHECK: [[mul4:%.*]] = [[MUL]] [[ELTY]] [[val4]], [[val3]]
  res[4] = things[3] * things[4];

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 5
  // CHECK: [[ld5:%.*]] = load [[TYPE]], [[TYPE]]* [[adr5]]
  // CHECK: [[val5:%.*]] = extractelement [[TYPE]] [[ld5]], i32 0
  // CHECK: [[div5:%.*]] = [[DIV]] [[ELTY]] [[val4]], [[val5]]
  res[5] = things[4] / things[5];

#ifndef DBL
  // NODBL: [[adr6:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 6
  // NODBL: [[ld6:%.*]] = load [[TYPE]], [[TYPE]]* [[adr6]]
  // NODBL: [[val6:%.*]] = extractelement [[TYPE]] [[ld6]]
  // NODBL: [[rem6:%.*]] = [[REM]] [[ELTY]] [[val5]], [[val6]]
  res[6] = things[5] % things[6];
#endif

  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 7
  // CHECK: [[ld7:%.*]] = load [[TYPE]], [[TYPE]]* [[adr7]]
  // CHECK: [[val7:%.*]] = extractelement [[TYPE]] [[ld7]], i32 0
  // CHECK: [[add7:%.*]] = [[ADD]] [[ELTY]] [[val7]], [[POS1:(1|1\.0*e\+0*|0xH3C00)]]
  // CHECK: [[res7:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add7]], i32 0
  // CHECK: store [[TYPE]] [[res7]], [[TYPE]]* [[adr7]]
  res[7] = things[7]++;

  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 8
  // CHECK: [[ld8:%.*]] = load [[TYPE]], [[TYPE]]* [[adr8]]
  // CHECK: [[val8:%.*]] = extractelement [[TYPE]] [[ld8]], i32 0
  // CHECK: [[add8:%.*]] = [[ADD]] [[ELTY]] [[val8]], [[NEG1:(-1|-1\.0*e\+0*|0xHBC00)]]
  // CHECK: [[res8:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add8]], i32 0
  // CHECK: store [[TYPE]] [[res8]], [[TYPE]]* [[adr8]]
  res[8] = things[8]--;

  // CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 9
  // CHECK: [[ld9:%.*]] = load [[TYPE]], [[TYPE]]* [[adr9]]
  // CHECK: [[val9:%.*]] = extractelement [[TYPE]] [[ld9]], i32 0
  // CHECK: [[add9:%.*]] = [[ADD]] [[ELTY]] [[val9]], [[POS1]]
  // CHECK: [[res9:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add9]], i32 0
  // CHECK: store [[TYPE]] [[res9]], [[TYPE]]* [[adr9]]
  res[9] = ++things[9];

  // CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %things, i32 0, i32 10
  // CHECK: [[ld10:%.*]] = load [[TYPE]], [[TYPE]]* [[adr10]]
  // CHECK: [[val10:%.*]] = extractelement [[TYPE]] [[ld10]], i32 0
  // CHECK: [[add10:%.*]] = [[ADD]] [[ELTY]] [[val10]], [[NEG1]]
  // CHECK: [[res10:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add10]], i32 0
  // CHECK: store [[TYPE]] [[res10]], [[TYPE]]* [[adr10]]
  res[10] = --things[10];

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 0
  // CHECK: store [[TYPE]] [[res0]], [[TYPE]]* [[adr0]]
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 1
  // CHECK: [[res1:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[sub1]], i64 0
  // CHECK: store [[TYPE]] [[res1]], [[TYPE]]* [[adr1]]
  // CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 2
  // CHECK: [[res2:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add2]], i64 0
  // CHECK: store [[TYPE]] [[res2]], [[TYPE]]* [[adr2]]
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 3
  // CHECK: [[res3:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[sub3]], i64 0
  // CHECK: store [[TYPE]] [[res3]], [[TYPE]]* [[adr3]]
  // CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 4
  // CHECK: [[res4:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[mul4]], i64 0
  // CHECK: store [[TYPE]] [[res4]], [[TYPE]]* [[adr4]]
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 5
  // CHECK: [[res5:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[div5]], i64 0
  // CHECK: store [[TYPE]] [[res5]], [[TYPE]]* [[adr5]]
  // NODBL: [[adr6:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 6
  // NODBL: [[res6:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[rem6]], i64 0
  // NODBL: store [[TYPE]] [[res6]], [[TYPE]]* [[adr6]]
  // CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 7
  // This is a post op, so the original value goes into res[].
  // CHECK: store [[TYPE]] [[ld7]], [[TYPE]]* [[adr7]]
  // CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 8
  // This is a post op, so the original value goes into res[].
  // CHECK: store [[TYPE]] [[ld8]], [[TYPE]]* [[adr8]]
  // CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 9
  // CHECK: [[res9:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add9]], i64 0
  // CHECK: store [[TYPE]] [[res9]], [[TYPE]]* [[adr9]]
  // CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x [[TYPE]]], [11 x [[TYPE]]]* %agg.result, i32 0, i32 10
  // CHECK: [[res10:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[add10]], i64 0
  // CHECK: store [[TYPE]] [[res10]], [[TYPE]]* [[adr10]]
  // CHECK: ret void
  return res;
}

// Test logic operators.
// Only permissable in pre-HLSL2021
// CHECK-LABEL: define void @"\01?logic
export bool logic(bool truth[10], VTYPE consequences[10])[10] {
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

  // CHECK: [[bval2:%.*]] = icmp ne i32 [[val2]], 0
  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 3
  // CHECK: [[val3:%.*]] = load i32, i32* [[adr3]]
  // CHECK: [[bval3:%.*]] = icmp ne i32 [[val3]], 0
  // CHECK: [[bres2:%.*]] = and i1 [[bval2]], [[bval3]]
  // CHECK: [[res2:%.*]] = zext i1 [[bres2]] to i32
  res[2] = truth[2] && truth[3];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 4
  // CHECK: [[val4:%.*]] = load i32, i32* [[adr4]]
  // CHECK: [[bval4:%.*]] = icmp ne i32 [[val4]], 0
  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 5
  // CHECK: [[val5:%.*]] = load i32, i32* [[adr5]]
  // CHECK: [[bval5:%.*]] = icmp ne i32 [[val5]], 0
  // CHECK: [[bres3:%.*]] = select i1 [[bval3]], i1 [[bval4]], i1 [[bval5]]
  // CHECK: [[res3:%.*]] = zext i1 [[bres3]] to i32
  res[3] = truth[3] ? truth[4] : truth[5];

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 0
  // CHECK: [[ld0:%.*]] = load [[TYPE]], [[TYPE]]* [[adr0]]
  // CHECK: [[val0:%.*]] = extractelement [[TYPE]] [[ld0]], i32 0
  // CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 1
  // CHECK: [[ld1:%.*]] = load [[TYPE]], [[TYPE]]* [[adr1]]
  // CHECK: [[val1:%.*]] = extractelement [[TYPE]] [[ld1]], i32 0
  // CHECK: [[cmp4:%.*]] = [[CMP:[fi]?cmp( fast)?]] {{o?}}eq [[ELTY]] [[val0]], [[val1]]
  // CHECK: [[res4:%.*]] = zext i1 [[cmp4]] to i32
  res[4] = consequences[0] == consequences[1];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 2
  // CHECK: [[ld2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[val2:%.*]] = extractelement [[TYPE]] [[ld2]], i32 0
  // CHECK: [[cmp5:%.*]] = [[CMP]] {{u?}}ne [[ELTY]] [[val1]], [[val2]]
  // CHECK: [[res5:%.*]] = zext i1 [[cmp5]] to i32
  res[5] = consequences[1] != consequences[2];

  // CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 3
  // CHECK: [[ld3:%.*]] = load [[TYPE]], [[TYPE]]* [[adr3]]
  // CHECK: [[val3:%.*]] = extractelement [[TYPE]] [[ld3]], i32 0
  // CHECK: [[cmp6:%.*]] = [[CMP]] {{[osu]?}}lt [[ELTY]] [[val2]], [[val3]]
  // CHECK: [[res6:%.*]] = zext i1 [[cmp6]] to i32
  res[6] = consequences[2] <  consequences[3];

  // CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 4
  // CHECK: [[ld4:%.*]] = load [[TYPE]], [[TYPE]]* [[adr4]]
  // CHECK: [[val4:%.*]] = extractelement [[TYPE]] [[ld4]], i32 0
  // CHECK: [[cmp7:%.*]] = [[CMP]] {{[osu]]?}}gt [[ELTY]] [[val3]], [[val4]]
  // CHECK: [[res7:%.*]] = zext i1 [[cmp7]] to i32
  res[7] = consequences[3] >  consequences[4];

  // CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 5
  // CHECK: [[ld5:%.*]] = load [[TYPE]], [[TYPE]]* [[adr5]]
  // CHECK: [[val5:%.*]] = extractelement [[TYPE]] [[ld5]], i32 0
  // CHECK: [[cmp8:%.*]] = [[CMP]] {{[osu]]?}}le [[ELTY]] [[val4]], [[val5]]
  // CHECK: [[res8:%.*]] = zext i1 [[cmp8]] to i32
  res[8] = consequences[4] <= consequences[5];

  // CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %consequences, i32 0, i32 6
  // CHECK: [[ld6:%.*]] = load [[TYPE]], [[TYPE]]* [[adr6]]
  // CHECK: [[val6:%.*]] = extractelement [[TYPE]] [[ld6]], i32 0
  // CHECK: [[cmp9:%.*]] = [[CMP]] {{[osu]?}}ge [[ELTY]] [[val5]], [[val6]]
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
export VTYPE index(VTYPE things[10], int i)[10] {
  // CHECK: [[res:%.*]] = alloca [10 x [[ELTY]]]
  VTYPE res[10];

  // CHECK: [[res0:%.*]] = getelementptr [10 x [[ELTY]]], [10 x [[ELTY]]]* [[res]], i32 0, i32 0
  // CHECK: store [[ELTY]] {{(0|0*\.?0*e?\+?0*|0xH0000)}}, [[ELTY]]* [[res0]]
  res[0] = 0;

  // CHECK: [[adri:%.*]] = getelementptr [10 x [[ELTY]]], [10 x [[ELTY]]]* [[res]], i32 0, i32 %i
  // CHECK: store [[ELTY]] [[POS1]], [[ELTY]]* [[adri]]
  res[i] = 1;

  // CHECK: [[adr2:%.*]] = getelementptr [10 x [[ELTY]]], [10 x [[ELTY]]]* [[res]], i32 0, i32 2
  // CHECK: store [[ELTY]] {{(2|2\.?0*e?\+?0*|0xH4000)}}, [[ELTY]]* [[adr2]]
  res[Ix] = 2;

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 0
  // CHECK: [[ld0:%.*]] = load [[TYPE]], [[TYPE]]* [[adr0]]
  // CHECK: [[adr3:%.*]] = getelementptr [10 x [[ELTY]]], [10 x [[ELTY]]]* [[res]], i32 0, i32 3
  // CHECK: [[thg0:%.*]] = extractelement [[TYPE]] [[ld0]], i64 0
  // CHECK: store [[ELTY]] [[thg0]], [[ELTY]]* [[adr3]]
  res[3] = things[0];

  // CHECK: [[adri:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 %i
  // CHECK: [[ldi:%.*]] = load [[TYPE]], [[TYPE]]* [[adri]]
  // CHECK: [[adr4:%.*]] = getelementptr [10 x [[ELTY]]], [10 x [[ELTY]]]* [[res]], i32 0, i32 4
  // CHECK: [[thgi:%.*]] = extractelement [[TYPE]] [[ldi]], i64 0
  // CHECK: store [[ELTY]] [[thgi]], [[ELTY]]* [[adr4]]
  res[4] = things[i];

  // CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x [[TYPE]]], [10 x [[TYPE]]]* %things, i32 0, i32 2
  // CHECK: [[ld2:%.*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // CHECK: [[adr5:%.*]] = getelementptr [10 x [[ELTY]]], [10 x [[ELTY]]]* [[res]], i32 0, i32 5
  // CHECK: [[thg2:%.*]] = extractelement [[TYPE]] [[ld2]], i64 0
  // CHECK: store [[ELTY]] [[thg2]], [[ELTY]]* [[adr5]]
  res[5] = things[Ix];
  // CHECK: ret void
  return res;
}

#ifdef INT
// Test bit twiddling operators.
// INT-LABEL: define void @"\01?bittwiddlers
export void bittwiddlers(inout VTYPE things[13]) {
  // INT: [[adr1:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 1
  // INT: [[ld1:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr1]]
  // INT: [[val1:%[0-9]*]] = extractelement [[TYPE]] [[ld1]], i32 0
  // INT: [[xor1:%[0-9]*]] = xor [[ELTY]] [[val1]], -1
  // INT: [[res1:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[xor1]], i32 0
  // INT: [[adr0:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 0
  // INT: store [[TYPE]] [[res1]], [[TYPE]]* [[adr0]]
  things[0] = ~things[1];

  // INT: [[adr2:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 2
  // INT: [[ld2:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr2]]
  // INT: [[val2:%[0-9]*]] = extractelement [[TYPE]] [[ld2]], i32 0
  // INT: [[adr3:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 3
  // INT: [[ld3:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr3]]
  // INT: [[val3:%[0-9]*]] = extractelement [[TYPE]] [[ld3]], i32 0
  // INT: [[or1:%[0-9]*]] = or [[ELTY]] [[val3]], [[val2]]
  // INT: [[res1:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[or1]], i32 0
  // INT: store [[TYPE]] [[res1]], [[TYPE]]* [[adr1]]
  things[1] = things[2] | things[3];

  // INT: [[adr4:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 4
  // INT: [[ld4:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr4]]
  // INT: [[val4:%[0-9]*]] = extractelement [[TYPE]] [[ld4]], i32 0
  // INT: [[and2:%[0-9]*]] = and [[ELTY]] [[val4]], [[val3]]
  // INT: [[res2:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[and2]], i32 0
  // INT: store [[TYPE]] [[res2]], [[TYPE]]* [[adr2]]
  things[2] = things[3] & things[4];

  // INT: [[adr5:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 5
  // INT: [[ld5:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr5]]
  // INT: [[val5:%[0-9]*]] = extractelement [[TYPE]] [[ld5]], i32 0
  // INT: [[xor3:%[0-9]*]] = xor [[ELTY]] [[val5]], [[val4]]
  // INT: [[res3:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[xor3]], i32 0
  // INT: store [[TYPE]] [[res3]], [[TYPE]]* [[adr3]]
  things[3] = things[4] ^ things[5];

  // INT: [[adr6:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 6
  // INT: [[ld6:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr6]]
  // INT: [[val6:%[0-9]*]] = extractelement [[TYPE]] [[ld6]], i32 0
  // INT: [[shv6:%[0-9]*]] = and [[ELTY]] [[val6]]
  // INT: [[shl4:%[0-9]*]] = shl [[ELTY]] [[val5]], [[shv6]]
  // INT: [[res4:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[shl4]], i32 0
  // INT: store [[TYPE]] [[res4]], [[TYPE]]* [[adr4]]
  things[4] = things[5] << things[6];

  // INT: [[adr7:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 7
  // INT: [[ld7:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr7]]
  // INT: [[val7:%[0-9]*]] = extractelement [[TYPE]] [[ld7]], i32 0
  // INT: [[shv7:%[0-9]*]] = and [[ELTY]] [[val7]]
  // UNSIG: [[shr5:%[0-9]*]] = lshr [[ELTY]] [[val6]], [[shv7]]
  // SIG: [[shr5:%[0-9]*]] = ashr [[ELTY]] [[val6]], [[shv7]]
  // INT: [[res5:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[shr5]], i32 0
  // INT: store [[TYPE]] [[res5]], [[TYPE]]* [[adr5]]
  things[5] = things[6] >> things[7];

  // INT: [[adr8:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 8
  // INT: [[ld8:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr8]]
  // INT: [[val8:%[0-9]*]] = extractelement [[TYPE]] [[ld8]], i32 0
  // INT: [[or6:%[0-9]*]] = or [[ELTY]] [[val8]], [[val6]]
  // INT: [[res6:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[or6]], i32 0
  // INT: store [[TYPE]] [[res6]], [[TYPE]]* [[adr6]]
  things[6] |= things[8];

  // INT: [[adr9:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 9
  // INT: [[ld9:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr9]]
  // INT: [[val9:%[0-9]*]] = extractelement [[TYPE]] [[ld9]], i32 0
  // INT: [[and7:%[0-9]*]] = and [[ELTY]] [[val9]], [[val7]]
  // INT: [[res7:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[and7]], i32 0
  // INT: store [[TYPE]] [[res7]], [[TYPE]]* [[adr7]]
  things[7] &= things[9];

  // INT: [[adr10:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 10
  // INT: [[ld10:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr10]]
  // INT: [[val10:%[0-9]*]] = extractelement [[TYPE]] [[ld10]], i32 0
  // INT: [[xor8:%[0-9]*]] = xor [[ELTY]] [[val10]], [[val8]]
  // INT: [[res8:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[xor8]], i32 0
  // INT: store [[TYPE]] [[res8]], [[TYPE]]* [[adr8]]
  things[8] ^= things[10];

  // INT: [[adr11:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 11
  // INT: [[ld11:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr11]]
  // INT: [[val11:%[0-9]*]] = extractelement [[TYPE]] [[ld11]], i32 0
  // INT: [[shv11:%[0-9]*]] = and [[ELTY]] [[val11]]
  // INT: [[shl9:%[0-9]*]] = shl [[ELTY]] [[val9]], [[shv11]]
  // INT: [[res9:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[shl9]], i32 0
  // INT: store [[TYPE]] [[res9]], [[TYPE]]* [[adr9]]
  things[9] <<= things[11];

  // INT: [[adr12:%[0-9]*]] = getelementptr inbounds [13 x [[TYPE]]], [13 x [[TYPE]]]* %things, i32 0, i32 12
  // INT: [[ld12:%[0-9]*]] = load [[TYPE]], [[TYPE]]* [[adr12]]
  // INT: [[val12:%[0-9]*]] = extractelement [[TYPE]] [[ld12]], i32 0
  // INT: [[shv12:%[0-9]*]] = and [[ELTY]] [[val12]]
  // UNSIG: [[shr10:%[0-9]*]] = lshr [[ELTY]] [[val10]], [[shv12]]
  // SIG: [[shr10:%[0-9]*]] = ashr [[ELTY]] [[val10]], [[shv12]]
  // INT: [[res10:%.*]] = insertelement [[TYPE]] undef, [[ELTY]] [[shr10]], i32 0
  // INT: store [[TYPE]] [[res10]], [[TYPE]]* [[adr10]]
  things[10] >>= things[12];

  // INT: ret void
}
#endif // INT
