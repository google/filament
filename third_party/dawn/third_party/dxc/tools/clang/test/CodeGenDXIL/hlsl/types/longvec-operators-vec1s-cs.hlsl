// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=float          %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=int      -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=double   -DDBL %s | FileCheck %s
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=uint64_t -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=float16_t      -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=int16_t  -DINT -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG

// Scalar variants to confirm they match.
// RUN: %dxc -DSCL -HV 2018 -T cs_6_9 -DTYPE=float          %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -DSCL -HV 2018 -T cs_6_9 -DTYPE=int      -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG
// RUN: %dxc -DSCL -HV 2018 -T cs_6_9 -DTYPE=double   -DDBL %s | FileCheck %s
// RUN: %dxc -DSCL -HV 2018 -T cs_6_9 -DTYPE=uint64_t -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG
// RUN: %dxc -DSCL -HV 2018 -T cs_6_9 -DTYPE=float16_t      -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL
// RUN: %dxc -DSCL -HV 2018 -T cs_6_9 -DTYPE=int16_t  -DINT -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG

// Linking tests.
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -Fo %t.1 %s
// RUN: %dxl -T cs_6_9 %t.1 | FileCheck %s --check-prefixes=CHECK,NODBL,NOINT
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=double -DDBL -Fo %t.2 %s
// RUN: %dxl -T cs_6_9 %t.2 | FileCheck %s --check-prefixes=CHECK,DBL,NOINT
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint16_t -DINT -enable-16bit-types -Fo %t.3 %s
// RUN: %dxl -T cs_6_9 %t.3 | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG

// Test relevant operators on vec1s in a 6.9 compute shader to ensure they continue to be treated as scalars.

// Just a trick to capture the needed type spellings since the DXC version of FileCheck can't do that explicitly.
// CHECK-DAG: %dx.types.ResRet.[[TY:[a-z][0-9]*]] = type { [[TYPE:[a-z0-9_]*]]
// CHECK-DAG: %dx.types.ResRet.[[ITY:i32]] = type { i32

#ifdef SCL
#define VTYPE TYPE
#else
#define VTYPE vector<TYPE, 1>
#endif

void assignments(inout VTYPE things[11], TYPE scales[10]);
VTYPE arithmetic(inout VTYPE things[11])[11];
VTYPE scarithmetic(VTYPE things[11], TYPE scales[10])[11];
bool1 logic(bool1 truth[10], VTYPE consequences[11])[10];
VTYPE index(VTYPE things[11], int i)[11];
void bittwiddlers(inout VTYPE things[13]);

struct Viface {
  VTYPE values[11];
};

struct Siface {
  TYPE values[10];
};

struct Liface {
  bool1 values[10];
};

struct Binface {
  VTYPE values[13];
};

RWStructuredBuffer<Viface> Input  : register(u11);
RWStructuredBuffer<Viface> Output : register(u12);
RWStructuredBuffer<Siface> Scales : register(u13);
RWStructuredBuffer<Liface> Truths : register(u14);
RWStructuredBuffer<Binface> Bits  : register(u15);
RWStructuredBuffer<vector<uint,13> > Offsets : register(u16);

[shader("compute")]
[numthreads(8,1,1)]
// CHECK-LABEL: define void @main
void main(uint3 GID : SV_GroupThreadID) {

  // CHECK-DAG: [[Input:%.*]]  = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 11, i32 11, i32 0, i8 1 }, i32 11
  // CHECK-DAG: [[Output:%.*]]  = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 12, i32 12, i32 0, i8 1 }, i32 12
  // CHECK-DAG: [[Scales:%.*]]  = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 13, i32 13, i32 0, i8 1 }, i32 13
  // CHECK-DAG: [[Truths:%.*]]  = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 14, i32 14, i32 0, i8 1 }, i32 14
  // INT-DAG: [[Bits:%.*]]  = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 15, i32 15, i32 0, i8 1 }, i32 15

  // CHECK: [[InIx1:%.*]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)
  // CHECK: [[InIx2:%.*]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 1)
  // CHECK: [[OutIx:%.*]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 2)

  uint InIx1 = GID[0];
  uint InIx2 = GID[1];
  uint OutIx = GID[2];

  // Assign vector offsets to capture the expected values.
  // CHECK: call void @dx.op.rawBufferVectorStore.v13i32(i32 304, %dx.types.Handle {{%.*}}, i32 0, i32 0, <13 x i32> <i32 [[OFF0:[0-9]*]], i32 [[OFF1:[0-9]*]], i32 [[OFF2:[0-9]*]], i32 [[OFF3:[0-9]*]], i32 [[OFF4:[0-9]*]], i32 [[OFF5:[0-9]*]], i32 [[OFF6:[0-9]*]], i32 [[OFF7:[0-9]*]], i32 [[OFF8:[0-9]*]], i32 [[OFF9:[0-9]*]], i32 [[OFF10:[0-9]*]], i32 [[OFF11:[0-9]*]], i32 [[OFF12:[0-9]*]]>
  Offsets[0] = vector<uint,13>(sizeof(TYPE)*0,
                               sizeof(TYPE)*1,
                               sizeof(TYPE)*2,
                               sizeof(TYPE)*3,
                               sizeof(TYPE)*4,
                               sizeof(TYPE)*5,
                               sizeof(TYPE)*6,
                               sizeof(TYPE)*7,
                               sizeof(TYPE)*8,
                               sizeof(TYPE)*9,
                               sizeof(TYPE)*10,
                               sizeof(TYPE)*11,
                               sizeof(TYPE)*12);

  // Assign boolean offsets to capture the expected values.
  // CHECK: call void @dx.op.rawBufferVectorStore.v13i32(i32 304, %dx.types.Handle {{%.*}}, i32 1, i32 0, <13 x i32> <i32 [[BOFF0:[0-9]*]], i32 [[BOFF1:[0-9]*]], i32 [[BOFF2:[0-9]*]], i32 [[BOFF3:[0-9]*]], i32 [[BOFF4:[0-9]*]], i32 [[BOFF5:[0-9]*]], i32 [[BOFF6:[0-9]*]], i32 [[BOFF7:[0-9]*]], i32 [[BOFF8:[0-9]*]], i32 [[BOFF9:[0-9]*]], i32 [[BOFF10:[0-9]*]], i32 [[ALN:[0-9]*]], i32 [[IALN:[0-9]*]]>
  Offsets[1] = vector<uint,13>(sizeof(int)*0,
                               sizeof(int)*1,
                               sizeof(int)*2,
                               sizeof(int)*3,
                               sizeof(int)*4,
                               sizeof(int)*5,
                               sizeof(int)*6,
                               sizeof(int)*7,
                               sizeof(int)*8,
                               sizeof(int)*9,
                               sizeof(int)*10,
                               sizeof(TYPE),// Effectively alignof.
                               sizeof(int));// Effectively integer alignof.

  assignments(Input[InIx1+1].values, Scales[InIx2+1].values);
  Output[OutIx+2].values = arithmetic(Input[InIx1+2].values);
  Output[OutIx+3].values = scarithmetic(Input[InIx1+3].values, Scales[InIx2+3].values);
  Truths[OutIx+4].values = logic(Truths[InIx2+4].values, Input[InIx1+4].values);
  Output[OutIx+5].values = index(Input[InIx1+5].values, InIx2+5);
#ifdef INT
  bittwiddlers(Bits[InIx1+6].values);
#endif
}
// A mixed-type overload to test overload resolution and mingle different vector element types in ops
// Test assignment operators.
void assignments(inout VTYPE things[11], TYPE scales[10]) {

  // CHECK: [[InIx:%.*]] = add i32 [[InIx1]], 1

  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF5]], i8 1, i32 [[ALN]])
  // CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF6]], i8 1, i32 [[ALN]])
  // CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF7]], i8 1, i32 [[ALN]])
  // CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF8]], i8 1, i32 [[ALN]])
  // CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF9]], i8 1, i32 [[ALN]])
  // CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF10]], i8 1, i32 [[ALN]])
  // CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0


  // CHECK: [[ScIx:%.*]] = add i32 [[InIx2]], 1
  // CHECK: [[ScHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Scales]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[scl0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // Nothing to check. Just a copy over.
  things[0] = scales[0];

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[scl1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[scl2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[scl3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[scl4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // CHECK: [[res1:%.*]] = [[ADD:f?add( fast)?]]{{( nsw)?}} [[TYPE]] [[val5]], [[val1]]
  things[1] += things[5];

  // CHECK: [[res2:%.*]] = [[SUB:f?sub( fast)?]]{{( nsw)?}} [[TYPE]] [[val2]], [[val6]]
  things[2] -= things[6];

  // CHECK: [[res3:%.*]] = [[MUL:f?mul( fast)?]]{{( nsw)?}} [[TYPE]] [[val7]], [[val3]]
  things[3] *= things[7];

  // CHECK: [[res4:%.*]] = [[DIV:[ufs]?div( fast)?]]{{( nsw)?}} [[TYPE]] [[val4]], [[val8]]
  things[4] /= things[8];

#ifdef DBL
  things[5] = 0; // Gotta give it something in any case for validation.
#else
  // NODBL: [[res5:%.*]] = [[REM:[ufs]?rem( fast)?]] [[TYPE]] [[val5]], [[val9]]
  things[5] %= things[9];
#endif

  // CHECK: [[res6:%[0-9]*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[scl1]], [[val6]]
  things[6] += scales[1];

  // CHECK: [[res7:%[0-9]*]] = [[SUB]]{{( nsw)?}} [[TYPE]] [[val7]], [[scl2]]
  things[7] -= scales[2];

  // CHECK: [[res8:%[0-9]*]] = [[MUL]]{{( nsw)?}} [[TYPE]] [[scl3]], [[val8]]
  things[8] *= scales[3];

  // CHECK: [[res9:%[0-9]*]] = [[DIV]]{{( nsw)?}} [[TYPE]] [[val9]], [[scl4]]
  things[9] /= scales[4];

  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF0]], [[TYPE]] [[scl0]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF1]], [[TYPE]] [[res1]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF2]], [[TYPE]] [[res2]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF3]], [[TYPE]] [[res3]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF4]], [[TYPE]] [[res4]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // NODBL: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF5]], [[TYPE]] [[res5]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF6]], [[TYPE]] [[res6]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF7]], [[TYPE]] [[res7]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF8]], [[TYPE]] [[res8]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF9]], [[TYPE]] [[res9]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF10]], [[TYPE]] [[val10]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])

}

// Test arithmetic operators.
VTYPE arithmetic(inout VTYPE things[11])[11] {
  TYPE res[11];
  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 2
  // CHECK: [[ResHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Output]]
  // CHECK: [[InIx:%.*]] = add i32 [[InIx1]], 2
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  res[0] = +things[0];

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF5]], i8 1, i32 [[ALN]])
  // CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF6]], i8 1, i32 [[ALN]])
  // CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF7]], i8 1, i32 [[ALN]])
  // CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF8]], i8 1, i32 [[ALN]])
  // CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF9]], i8 1, i32 [[ALN]])
  // CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF10]], i8 1, i32 [[ALN]])
  // CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0


  // CHECK: [[res1:%.*]] = [[SUB]]{{( nsw)?}} [[TYPE]] {{-?(0|0\.?0*e?\+?0*|0xH8000)}}, [[val0]]
  res[1] = -things[0];

  // CHECK: [[res2:%.*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[val2]], [[val1]]
  res[2] = things[1] + things[2];

  // CHECK: [[res3:%.*]] = [[SUB]]{{( nsw)?}} [[TYPE]] [[val2]], [[val3]]
  res[3] = things[2] - things[3];

  // CHECK: [[res4:%.*]] = [[MUL]]{{( nsw)?}} [[TYPE]] [[val4]], [[val3]]
  res[4] = things[3] * things[4];

  // CHECK: [[res5:%.*]] = [[DIV]]{{( nsw)?}} [[TYPE]] [[val4]], [[val5]]
  res[5] = things[4] / things[5];

#ifdef DBL
  res[6] = 0; // Gotta give it something in any case for validation.
#else
  // NODBL: [[res6:%.*]] = [[REM]] [[TYPE]] [[val5]], [[val6]]
  res[6] = things[5] % things[6];
#endif

  // CHECK: [[res7:%[0-9]*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[val7]], [[POS1:(1|1\.0*e\+0*|0xH3C00)]]
  res[7] = things[7]++;

  // CHECK: [[res8:%[0-9]*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[val8]], [[NEG1:(-1|-1\.0*e\+0*|0xHBC00)]]
  res[8] = things[8]--;

  // CHECK: [[res9:%.*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[val9]], [[POS1]]
  res[9] = ++things[9];

  // CHECK: [[res10:%.*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[val10]], [[NEG1]]
  res[10] = --things[10];

  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF0]], [[TYPE]] [[val0]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF1]], [[TYPE]] [[val1]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF2]], [[TYPE]] [[val2]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF3]], [[TYPE]] [[val3]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF4]], [[TYPE]] [[val4]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF5]], [[TYPE]] [[val5]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF6]], [[TYPE]] [[val6]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF7]], [[TYPE]] [[res7]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF8]], [[TYPE]] [[res8]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF9]], [[TYPE]] [[res9]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF10]], [[TYPE]] [[res10]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])

  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF0]], [[TYPE]] [[val0]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF1]], [[TYPE]] [[res1]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF2]], [[TYPE]] [[res2]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF3]], [[TYPE]] [[res3]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF4]], [[TYPE]] [[res4]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF5]], [[TYPE]] [[res5]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // NODBL: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF6]], [[TYPE]] [[res6]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // Postincrement/decrements get the original value.
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF7]], [[TYPE]] [[val7]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF8]], [[TYPE]] [[val8]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF9]], [[TYPE]] [[res9]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF10]], [[TYPE]] [[res10]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])

  return res;
}

// Test arithmetic operators with scalars.
VTYPE scarithmetic(VTYPE things[11], TYPE scales[10])[11] {
  VTYPE res[11];

  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 3
  // CHECK: [[ResHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Output]]
  // CHECK: [[InIx:%.*]] = add i32 [[InIx1]], 3
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF5]], i8 1, i32 [[ALN]])
  // CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[InIx]], i32 [[OFF6]], i8 1, i32 [[ALN]])
  // CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // CHECK: [[SclIx:%.*]] = add i32 [[InIx2]], 3
  // CHECK: [[SclHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Scales]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[scl0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[scl1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[scl2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[scl3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[scl4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF5]], i8 1, i32 [[ALN]])
  // CHECK: [[scl5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF6]], i8 1, i32 [[ALN]])
  // CHECK: [[scl6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // CHECK: [[res0:%[0-9]*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[scl0]], [[val0]]
  res[0] = things[0] + scales[0];

  // CHECK: [[res1:%[0-9]*]] = [[SUB]]{{( nsw)?}} [[TYPE]] [[val1]], [[scl1]]
  res[1] = things[1] - scales[1];

  // CHECK: [[res2:%[0-9]*]] = [[MUL]]{{( nsw)?}} [[TYPE]] [[scl2]], [[val2]]
  res[2] = things[2] * scales[2];

  // CHECK: [[res3:%[0-9]*]] = [[DIV]]{{( nsw)?}} [[TYPE]] [[val3]], [[scl3]]
  res[3] = things[3] / scales[3];

  // CHECK: [[res4:%[0-9]*]] = [[ADD]]{{( nsw)?}} [[TYPE]] [[scl4]], [[val4]]
  res[4] = scales[4] + things[4];

  // CHECK: [[res5:%[0-9]*]] = [[SUB]]{{( nsw)?}} [[TYPE]] [[scl5]], [[val5]]
  res[5] = scales[5] - things[5];

  // CHECK: [[res6:%[0-9]*]] = [[MUL]]{{( nsw)?}} [[TYPE]] [[scl6]], [[val6]]
  res[6] = scales[6] * things[6];
  res[7] = res[8] = res[9] = res[10] = 0;

  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF0]], [[TYPE]] [[res0]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF1]], [[TYPE]] [[res1]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF2]], [[TYPE]] [[res2]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF3]], [[TYPE]] [[res3]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF4]], [[TYPE]] [[res4]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF5]], [[TYPE]] [[res5]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF6]], [[TYPE]] [[res6]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])

  return res;
}


// Test logic operators.
// Only permissable in pre-HLSL2021
bool1 logic(bool1 truth[10], VTYPE consequences[11])[10] {
  bool1 res[10];

  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 4
  // CHECK: [[TruHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Truths]]
  // CHECK: [[TruIx:%.*]] = add i32 [[InIx2]], 4
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferLoad.[[ITY]](i32 139, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF0]], i8 1, i32 [[IALN]])
  // CHECK: [[ival0:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferLoad.[[ITY]](i32 139, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF1]], i8 1, i32 [[IALN]])
  // CHECK: [[ival1:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferLoad.[[ITY]](i32 139, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF2]], i8 1, i32 [[IALN]])
  // CHECK: [[ival2:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferLoad.[[ITY]](i32 139, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF3]], i8 1, i32 [[IALN]])
  // CHECK: [[ival3:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferLoad.[[ITY]](i32 139, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF4]], i8 1, i32 [[IALN]])
  // CHECK: [[ival4:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferLoad.[[ITY]](i32 139, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF5]], i8 1, i32 [[IALN]])
  // CHECK: [[ival5:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0

  // CHECK: [[valIx:%.*]] = add i32 [[InIx1]], 4
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF5]], i8 1, i32 [[ALN]])
  // CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF6]], i8 1, i32 [[ALN]])
  // CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0


  // CHECK: [[bres0:%.*]] = icmp eq i32 [[ival0]], 0
  // CHECK: [[res0:%.*]] = zext i1 [[bres0]] to i32
  res[0] = !truth[0];

  // CHECK: [[res1:%.*]] = or i32 [[ival2]], [[ival1]]
  // CHECK: [[bres1:%.*]] = icmp ne i32 [[res1]], 0
  // CHECK: [[res1:%.*]] = zext i1 [[bres1]] to i32
  res[1] = truth[1] || truth[2];

  // CHECK: [[bval2:%.*]] = icmp ne i32 [[ival2]], 0
  // CHECK: [[bval3:%.*]] = icmp ne i32 [[ival3]], 0
  // CHECK: [[bres2:%.*]] = and i1 [[bval2]], [[bval3]]
  // CHECK: [[res2:%.*]] = zext i1 [[bres2]] to i32
  res[2] = truth[2] && truth[3];

  // CHECK: [[bval4:%.*]] = icmp ne i32 [[ival4]], 0
  // CHECK: [[bval5:%.*]] = icmp ne i32 [[ival5]], 0
  // CHECK: [[bres3:%.*]] = select i1 [[bval3]], i1 [[bval4]], i1 [[bval5]]
  // CHECK: [[res3:%.*]] = zext i1 [[bres3]] to i32
  res[3] = truth[3] ? truth[4] : truth[5];

  // CHECK: [[cmp4:%.*]] = [[CMP:[fi]?cmp( fast)?]] {{o?}}eq [[TYPE]] [[val0]], [[val1]]
  // CHECK: [[res4:%.*]] = zext i1 [[cmp4]] to i32
  res[4] = consequences[0] == consequences[1];

  // CHECK: [[cmp5:%.*]] = [[CMP]] {{u?}}ne [[TYPE]] [[val1]], [[val2]]
  // CHECK: [[res5:%.*]] = zext i1 [[cmp5]] to i32
  res[5] = consequences[1] != consequences[2];

  // CHECK: [[cmp6:%.*]] = [[CMP]] {{[osu]?}}lt [[TYPE]] [[val2]], [[val3]]
  // CHECK: [[res6:%.*]] = zext i1 [[cmp6]] to i32
  res[6] = consequences[2] <  consequences[3];

  // CHECK: [[cmp7:%.*]] = [[CMP]] {{[osu]]?}}gt [[TYPE]] [[val3]], [[val4]]
  // CHECK: [[res7:%.*]] = zext i1 [[cmp7]] to i32
  res[7] = consequences[3] >  consequences[4];

  // CHECK: [[cmp8:%.*]] = [[CMP]] {{[osu]]?}}le [[TYPE]] [[val4]], [[val5]]
  // CHECK: [[res8:%.*]] = zext i1 [[cmp8]] to i32
  res[8] = consequences[4] <= consequences[5];

  // CHECK: [[cmp9:%.*]] = [[CMP]] {{[osu]?}}ge [[TYPE]] [[val5]], [[val6]]
  // CHECK: [[res9:%.*]] = zext i1 [[cmp9]] to i32
  res[9] = consequences[5] >= consequences[6];

  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF0]], i32 [[res0]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF1]], i32 [[res1]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF2]], i32 [[res2]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF3]], i32 [[res3]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF4]], i32 [[res4]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF5]], i32 [[res5]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF6]], i32 [[res6]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF7]], i32 [[res7]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF8]], i32 [[res8]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
  // CHECK: call void @dx.op.rawBufferStore.[[ITY]](i32 140, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF9]], i32 [[res9]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)

  return res;
}

static const int Ix = 2;

// Test indexing operators
VTYPE index(VTYPE things[11], int i)[11] {
  VTYPE res[11];

  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 5
  // CHECK: [[ResHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Output]]
  // CHECK: [[valIx:%.*]] = add i32 [[InIx1]], 5
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]

  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1:%.*]], i32 0, i32 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val0]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 1
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val1]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 2
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val2]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 3
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val3]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 4
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val4]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 5
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF5]], i8 1, i32 [[ALN]])
  // CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val5]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 6
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF6]], i8 1, i32 [[ALN]])
  // CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val6]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 7
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF7]], i8 1, i32 [[ALN]])
  // CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val7]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 8
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF8]], i8 1, i32 [[ALN]])
  // CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val8]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 9
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF9]], i8 1, i32 [[ALN]])
  // CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val9]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 10
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[valIx]], i32 [[OFF10]], i8 1, i32 [[ALN]])
  // CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store [[TYPE]] [[val10]], [[TYPE]]* [[adr]], align [[ALN]]

  // CHECK: [[Ix:%.*]] = add i32 [[InIx2]], 5

  // CHECK: [[adr0:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2:%.*]], i32 0, i32 0
  // CHECK: store [[TYPE]] {{(0|0\.?0*e?\+?0*|0xH0000)}}, [[TYPE]]* [[adr0]], align [[ALN]]
  res[0] = 0;

  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 [[Ix]]
  // CHECK: store [[TYPE]] [[POS1]], [[TYPE]]* [[adr]]
  res[i] = 1;

  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 2
  // CHECK: store [[TYPE]] [[TWO:(2|2\.?0*e?\+?0*|0xH4000)]], [[TYPE]]* [[adr]]
  res[Ix] = 2;

  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 3
  // CHECK: store [[TYPE]] [[val0]], [[TYPE]]* [[adr]]
  res[3] = things[0];

  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr1]], i32 0, i32 [[Ix]]
  // CHECK: [[vali:%.*]] = load [[TYPE]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 4
  // CHECK: store [[TYPE]] [[vali]], [[TYPE]]* [[adr]]
  res[4] = things[i];

  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 5
  // CHECK: store [[TYPE]] [[val2]], [[TYPE]]* [[adr]]
  res[5] = things[Ix];

  // CHECK: [[ld:%.*]] = load [[TYPE]], [[TYPE]]* [[adr0]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 0, [[TYPE]] [[ld]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 1
  // CHECK: [[ld:%.*]] = load [[TYPE]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF1]], [[TYPE]] [[ld]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF2]], [[TYPE]] [[TWO]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF3]], [[TYPE]] [[val0]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF4]], [[TYPE]] [[vali]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF5]], [[TYPE]] [[val2]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 6
  // CHECK: [[ld:%.*]] = load [[TYPE]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF6]], [[TYPE]] [[ld]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 7
  // CHECK: [[ld:%.*]] = load [[TYPE]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF7]], [[TYPE]] [[ld]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 8
  // CHECK: [[ld:%.*]] = load [[TYPE]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF8]], [[TYPE]] [[ld]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 9
  // CHECK: [[ld:%.*]] = load [[TYPE]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF9]], [[TYPE]] [[ld]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr{{( inbounds)?}} [11 x [[TYPE]]], [11 x [[TYPE]]]* [[scr2]], i32 0, i32 10
  // CHECK: [[ld:%.*]] = load [[TYPE]], [[TYPE]]* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF10]], [[TYPE]] [[ld]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])

  return res;
}

#ifdef INT
// Test bit twiddling operators.
void bittwiddlers(inout VTYPE things[13]) {
  // INT: [[ValIx:%.*]] = add i32 [[InIx1]], 6
  // INT: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Bits]]
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF1]], i8 1, i32 [[ALN]])
  // INT: [[val1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF2]], i8 1, i32 [[ALN]])
  // INT: [[val2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF3]], i8 1, i32 [[ALN]])
  // INT: [[val3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF4]], i8 1, i32 [[ALN]])
  // INT: [[val4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF5]], i8 1, i32 [[ALN]])
  // INT: [[val5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF6]], i8 1, i32 [[ALN]])
  // INT: [[val6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF7]], i8 1, i32 [[ALN]])
  // INT: [[val7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF8]], i8 1, i32 [[ALN]])
  // INT: [[val8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF9]], i8 1, i32 [[ALN]])
  // INT: [[val9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF10]], i8 1, i32 [[ALN]])
  // INT: [[val10:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF11]], i8 1, i32 [[ALN]])
  // INT: [[val11:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF12]], i8 1, i32 [[ALN]])
  // INT: [[val12:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // INT: [[res0:%[0-9]*]] = xor [[TYPE]] [[val1]], -1
  things[0] = ~things[1];

  // INT: [[res1:%[0-9]*]] = or [[TYPE]] [[val3]], [[val2]]
  things[1] = things[2] | things[3];

  // INT: [[res2:%[0-9]*]] = and [[TYPE]] [[val4]], [[val3]]
  things[2] = things[3] & things[4];

  // INT: [[res3:%[0-9]*]] = xor [[TYPE]] [[val5]], [[val4]]
  things[3] = things[4] ^ things[5];

  // INT: [[shv6:%[0-9]*]] = and [[TYPE]] [[val6]]
  // INT: [[res4:%[0-9]*]] = shl [[TYPE]] [[val5]], [[shv6]]
  things[4] = things[5] << things[6];

  // INT: [[shv7:%[0-9]*]] = and [[TYPE]] [[val7]]
  // UNSIG: [[res5:%[0-9]*]] = lshr [[TYPE]] [[val6]], [[shv7]]
  // SIG: [[res5:%[0-9]*]] = ashr [[TYPE]] [[val6]], [[shv7]]
  things[5] = things[6] >> things[7];

  // INT: [[res6:%[0-9]*]] = or [[TYPE]] [[val8]], [[val6]]
  things[6] |= things[8];

  // INT: [[res7:%[0-9]*]] = and [[TYPE]] [[val9]], [[val7]]
  things[7] &= things[9];

  // INT: [[res8:%[0-9]*]] = xor [[TYPE]] [[val10]], [[val8]]
  things[8] ^= things[10];

  // INT: [[shv11:%[0-9]*]] = and [[TYPE]] [[val11]]
  // INT: [[res9:%[0-9]*]] = shl [[TYPE]] [[val9]], [[shv11]]
  things[9] <<= things[11];

  // INT: [[shv12:%[0-9]*]] = and [[TYPE]] [[val12]]
  // UNSIG: [[res10:%[0-9]*]] = lshr [[TYPE]] [[val10]], [[shv12]]
  // SIG: [[res10:%[0-9]*]] = ashr [[TYPE]] [[val10]], [[shv12]]
  things[10] >>= things[12];

  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF0]], [[TYPE]] [[res0]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF1]], [[TYPE]] [[res1]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF2]], [[TYPE]] [[res2]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF3]], [[TYPE]] [[res3]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF4]], [[TYPE]] [[res4]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF5]], [[TYPE]] [[res5]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF6]], [[TYPE]] [[res6]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF7]], [[TYPE]] [[res7]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF8]], [[TYPE]] [[res8]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF9]], [[TYPE]] [[res9]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF10]], [[TYPE]] [[res10]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF11]], [[TYPE]] [[val11]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])
  // INT: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[InHdl]], i32 [[ValIx]], i32 [[OFF12]], [[TYPE]] [[val12]], [[TYPE]] undef, [[TYPE]] undef, [[TYPE]] undef, i8 1, i32 [[ALN]])

  // CHECK-LABEL: ret void
}
#endif // INT
