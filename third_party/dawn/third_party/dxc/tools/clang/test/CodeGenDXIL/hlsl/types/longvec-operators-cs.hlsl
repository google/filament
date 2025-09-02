// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=float    -DNUM=2 %s | FileCheck %s --check-prefixes=CHECK,NODBL,NOINT
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=float    -DNUM=17 %s | FileCheck %s --check-prefixes=CHECK,NODBL,NOINT
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=int      -DNUM=2 -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=uint     -DNUM=5 -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=double   -DNUM=3 -DDBL %s | FileCheck %s --check-prefixes=CHECK,DBL,NOINT
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=uint64_t -DNUM=9 -DINT %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=float16_t -DNUM=17 -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL,NOINT
// RUN: %dxc -HV 2018 -T cs_6_9 -DTYPE=int16_t   -DNUM=33 -DINT -enable-16bit-types %s | FileCheck %s --check-prefixes=CHECK,NODBL,INT,SIG

// Linking tests.
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=float -DNUM=6 -Fo %t.1 %s
// RUN: %dxl -T cs_6_9 %t.1 | FileCheck %s --check-prefixes=CHECK,NODBL,NOINT
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=double -DNUM=3 -DDBL -Fo %t.2 %s
// RUN: %dxl -T cs_6_9 %t.2 | FileCheck %s --check-prefixes=CHECK,DBL,NOINT
// RUN: %dxc -HV 2018 -T lib_6_9 -DTYPE=uint16_t -DNUM=12 -DINT -enable-16bit-types -Fo %t.3 %s
// RUN: %dxl -T cs_6_9 %t.3 | FileCheck %s --check-prefixes=CHECK,NODBL,INT,UNSIG

// Test relevant operators on an assortment vector sizes and types with 6.9 native vectors.
// Tests in a CS environment where vector operations were previously disallowed to confirm that they are retained.

// Just a trick to capture the needed type spellings since the DXC version of FileCheck can't do that explicitly.
// Uses non vector buffer to avoid interacting with that implementation.
// CHECK-DAG: %dx.types.ResRet.[[TY:v[0-9]*[a-z][0-9]*]] = type { <[[NUM:[0-9]*]] x [[TYPE:[a-z_0-9]*]]>
// CHECK-DAG: %dx.types.ResRet.[[STY:[a-z][0-9]*]] = type { [[STYPE:[a-z0-9_]*]]
// CHECK-DAG: %dx.types.ResRet.[[ITY:v[0-9]*i32]] = type { <[[NUM]] x i32>

void assignments(inout vector<TYPE, NUM> things[11], TYPE scales[10]);
vector<TYPE, NUM> arithmetic(inout vector<TYPE, NUM> things[11])[11];
vector<TYPE, NUM> scarithmetic(vector<TYPE, NUM> things[11], TYPE scales[10])[11];
vector<bool, NUM> logic(vector<bool, NUM> truth[10], vector<TYPE, NUM> consequences[11])[10];
vector<TYPE, NUM> index(vector<TYPE, NUM> things[11], int i)[11];
void bittwiddlers(inout vector<TYPE, NUM> things[13]);

struct Viface {
  vector<TYPE, NUM> values[11];
};

struct Siface {
  TYPE values[10];
};

struct Liface {
  vector<bool, NUM> values[10];
};

struct Binface {
  vector<TYPE, NUM> values[13];
};

RWStructuredBuffer<Viface> Input : register(u11);
RWStructuredBuffer<Viface> Output : register(u12);
RWStructuredBuffer<Siface> Scales : register(u13);
RWStructuredBuffer<Liface> Truths : register(u14);
RWStructuredBuffer<Binface> Bits : register(u15);
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
  // CHECK: [[scratch1:%.*]] = alloca [11 x <[[NUM]] x [[TYPE]]>]
  // CHECK: [[scratch2:%.*]] = alloca [11 x <[[NUM]] x [[TYPE]]>]

  uint InIx1 = GID[0];
  uint InIx2 = GID[1];
  uint OutIx = GID[2];

  // Assign vector offsets to capture the expected values.
  // CHECK: call void @dx.op.rawBufferVectorStore.v13i32(i32 304, %dx.types.Handle {{%.*}}, i32 0, i32 0, <13 x i32> <i32 [[OFF0:[0-9]*]], i32 [[OFF1:[0-9]*]], i32 [[OFF2:[0-9]*]], i32 [[OFF3:[0-9]*]], i32 [[OFF4:[0-9]*]], i32 [[OFF5:[0-9]*]], i32 [[OFF6:[0-9]*]], i32 [[OFF7:[0-9]*]], i32 [[OFF8:[0-9]*]], i32 [[OFF9:[0-9]*]], i32 [[OFF10:[0-9]*]], i32 [[OFF11:[0-9]*]], i32 [[OFF12:[0-9]*]]>
  Offsets[0] = vector<uint,13>(sizeof(vector<TYPE, NUM>)*0,
                               sizeof(vector<TYPE, NUM>)*1,
                               sizeof(vector<TYPE, NUM>)*2,
                               sizeof(vector<TYPE, NUM>)*3,
                               sizeof(vector<TYPE, NUM>)*4,
                               sizeof(vector<TYPE, NUM>)*5,
                               sizeof(vector<TYPE, NUM>)*6,
                               sizeof(vector<TYPE, NUM>)*7,
                               sizeof(vector<TYPE, NUM>)*8,
                               sizeof(vector<TYPE, NUM>)*9,
                               sizeof(vector<TYPE, NUM>)*10,
                               sizeof(vector<TYPE, NUM>)*11,
                               sizeof(vector<TYPE, NUM>)*12);

  // Assign scalar offsets to capture the expected values.
  // CHECK: call void @dx.op.rawBufferVectorStore.v13i32(i32 304, %dx.types.Handle {{%.*}}, i32 1, i32 0, <13 x i32> <i32 [[SOFF0:[0-9]*]], i32 [[SOFF1:[0-9]*]], i32 [[SOFF2:[0-9]*]], i32 [[SOFF3:[0-9]*]], i32 [[SOFF4:[0-9]*]], i32 [[SOFF5:[0-9]*]], i32 [[SOFF6:[0-9]*]], i32 [[SOFF7:[0-9]*]], i32 [[SOFF8:[0-9]*]], i32 [[SOFF9:[0-9]*]], i32 [[SOFF10:[0-9]*]], i32 [[ALN:[0-9]*]], i32 [[IALN:[0-9]*]]>
  Offsets[1] = vector<uint,13>(sizeof(TYPE)*0,
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
                               sizeof(TYPE),// Effectively alignof.
                               sizeof(int));// Effectively integer alignof.

  // Assign boolean offsets to capture the expected values.
  // CHECK: call void @dx.op.rawBufferVectorStore.v13i32(i32 304, %dx.types.Handle {{%.*}}, i32 2, i32 0, <13 x i32> <i32 [[BOFF0:[0-9]*]], i32 [[BOFF1:[0-9]*]], i32 [[BOFF2:[0-9]*]], i32 [[BOFF3:[0-9]*]], i32 [[BOFF4:[0-9]*]], i32 [[BOFF5:[0-9]*]], i32 [[BOFF6:[0-9]*]], i32 [[BOFF7:[0-9]*]], i32 [[BOFF8:[0-9]*]], i32 [[BOFF9:[0-9]*]], i32 [[BOFF10:[0-9]*]], i32 [[BOFF11:[0-9]*]], i32 [[BOFF12:[0-9]*]]>
  Offsets[2] = vector<uint,13>(sizeof(vector<int,NUM>)*0,
                               sizeof(vector<int,NUM>)*1,
                               sizeof(vector<int,NUM>)*2,
                               sizeof(vector<int,NUM>)*3,
                               sizeof(vector<int,NUM>)*4,
                               sizeof(vector<int,NUM>)*5,
                               sizeof(vector<int,NUM>)*6,
                               sizeof(vector<int,NUM>)*7,
                               sizeof(vector<int,NUM>)*8,
                               sizeof(vector<int,NUM>)*9,
                               sizeof(vector<int,NUM>)*10,
                               sizeof(vector<int,NUM>)*11,
                               sizeof(vector<int,NUM>)*12);

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
void assignments(inout vector<TYPE, NUM> things[11], TYPE scales[10]) {

  // CHECK: [[VcIx:%.*]] = add i32 [[InIx1]], 1
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF1]], i32 [[ALN]])
  // CHECK: [[vec1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF2]], i32 [[ALN]])
  // CHECK: [[vec2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF3]], i32 [[ALN]])
  // CHECK: [[vec3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF4]], i32 [[ALN]])
  // CHECK: [[vec4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF5]], i32 [[ALN]])
  // CHECK: [[vec5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF6]], i32 [[ALN]])
  // CHECK: [[vec6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF7]], i32 [[ALN]])
  // CHECK: [[vec7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF8]], i32 [[ALN]])
  // CHECK: [[vec8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF9]], i32 [[ALN]])
  // CHECK: [[vec9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // CHECK: [[ScIx:%.*]] = add i32 [[InIx2]], 1
  // CHECK: [[ScHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Scales]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[scl0:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[SOFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[scl1:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[SOFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[scl2:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[SOFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[scl3:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[ScHdl]], i32 [[ScIx]], i32 [[SOFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[scl4:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0


  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl0]], i32 0
  // CHECK: [[res0:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  things[0] = scales[0];

  // CHECK: [[res1:%[0-9]*]] = [[ADD:f?add( fast)?]] <[[NUM]] x [[TYPE]]> [[vec5]], [[vec1]]
  things[1] += things[5];

  // CHECK: [[res2:%[0-9]*]] = [[SUB:f?sub( fast)?]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec6]]
  things[2] -= things[6];

  // CHECK: [[res3:%[0-9]*]] = [[MUL:f?mul( fast)?]] <[[NUM]] x [[TYPE]]> [[vec7]], [[vec3]]
  things[3] *= things[7];

  // CHECK: [[res4:%[0-9]*]] = [[DIV:[ufs]?div( fast)?]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec8]]
  things[4] /= things[8];

#ifdef DBL
  // DBL can't use remainder operator, do something anyway to keep the rest consistent.
  // DBL: [[fvec9:%[0-9]*]] = fptrunc <[[NUM]] x double> [[vec9]] to <[[NUM]] x float>
  // DBL: [[fvec5:%[0-9]*]] = fptrunc <[[NUM]] x double> [[vec5]] to <[[NUM]] x float>
  // DBL: [[fres5:%[0-9]*]] = [[REM:[ufs]?rem( fast)?]] <[[NUM]] x float> [[fvec5]], [[fvec9]]
  // DBL: [[res5:%[0-9]*]] = fpext <[[NUM]] x float> [[fres5]] to <[[NUM]] x double>
  vector<float,NUM> f9 = (vector<float,NUM>)things[9];
  vector<float,NUM> f5 = (vector<float,NUM>)things[5];
  f5 %= f9;
  things[5] = f5;
#else
  // NODBL: [[res5:%[0-9]*]] = [[REM:[ufs]?rem( fast)?]] <[[NUM]] x [[TYPE]]> [[vec5]], [[vec9]]
  things[5] %= things[9];
#endif

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl1]], i32 0
  // CHECK: [[spt1:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res6:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[spt1]], [[vec6]]
  things[6] += scales[1];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl2]], i32 0
  // CHECK: [[spt2:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res7:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[vec7]], [[spt2]]
  things[7] -= scales[2];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl3]], i32 0
  // CHECK: [[spt3:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res8:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[spt3]], [[vec8]]
  things[8] *= scales[3];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl4]], i32 0
  // CHECK: [[spt4:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res9:%[0-9]*]] = [[DIV]] <[[NUM]] x [[TYPE]]> [[vec9]], [[spt4]]
  things[9] /= scales[4];

  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF0]], <[[NUM]] x [[TYPE]]> [[res0]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF1]], <[[NUM]] x [[TYPE]]> [[res1]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF2]], <[[NUM]] x [[TYPE]]> [[res2]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF3]], <[[NUM]] x [[TYPE]]> [[res3]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF4]], <[[NUM]] x [[TYPE]]> [[res4]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF5]], <[[NUM]] x [[TYPE]]> [[res5]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF6]], <[[NUM]] x [[TYPE]]> [[res6]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF7]], <[[NUM]] x [[TYPE]]> [[res7]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF8]], <[[NUM]] x [[TYPE]]> [[res8]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF9]], <[[NUM]] x [[TYPE]]> [[res9]], i32 [[ALN]])

}

// Test arithmetic operators.
vector<TYPE, NUM> arithmetic(inout vector<TYPE, NUM> things[11])[11] {
  vector<TYPE, NUM> res[11];

  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 2
  // CHECK: [[ResHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Output]]
  // CHECK: [[VecIx:%.*]] = add i32 [[InIx1]], 2
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF0]], i32 [[ALN]])
  // CHECK: [[vec0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF1]], i32 [[ALN]])
  // CHECK: [[vec1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF2]], i32 [[ALN]])
  // CHECK: [[vec2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF3]], i32 [[ALN]])
  // CHECK: [[vec3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF4]], i32 [[ALN]])
  // CHECK: [[vec4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF5]], i32 [[ALN]])
  // CHECK: [[vec5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF6]], i32 [[ALN]])
  // CHECK: [[vec6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF7]], i32 [[ALN]])
  // CHECK: [[vec7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF8]], i32 [[ALN]])
  // CHECK: [[vec8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF9]], i32 [[ALN]])
  // CHECK: [[vec9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF10]], i32 [[ALN]])
  // CHECK: [[vec10:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // NOINT: [[res0:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> <[[TYPE]] {{-?(0|0\.0*e\+0*|0xH8000),.*}}>, [[vec0]]
  // INT: [[res0:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> zeroinitializer, [[vec0]]
  res[0] = -things[0];
  res[1] = +things[0];

  // CHECK: [[res2:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec1]]
  res[2] = things[1] + things[2];

  // CHECK: [[res3:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[vec2]], [[vec3]]
  res[3] = things[2] - things[3];

  // CHECK: [[res4:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec3]]
  res[4] = things[3] * things[4];

  // CHECK: [[res5:%[0-9]*]] = [[DIV]] <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  res[5] = things[4] / things[5];

  // DBL: [[fvec5:%[0-9]*]] = fptrunc <[[NUM]] x double> [[vec5]] to <[[NUM]] x float>
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

  // CHECK: [[res7:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec7]], <[[TYPE]] [[POS1:(1|1\.0*e\+0*|0xH3C00)]]
  res[7] = things[7]++;

  // CHECK: [[res8:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec8]], <[[TYPE]] [[NEG1:(-1|-1\.0*e\+0*|0xHBC00)]]
  res[8] = things[8]--;

  // CHECK: [[res9:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec9]], <[[TYPE]] [[POS1]]
  res[9] = ++things[9];

  // CHECK: [[res10:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[vec10]], <[[TYPE]] [[NEG1]]
  res[10] = --things[10];

  // Things[] input gets all the result values since pre/post inc/decrements don't change the end result.
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF7]], <[[NUM]] x [[TYPE]]> [[res7]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF8]], <[[NUM]] x [[TYPE]]> [[res8]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF9]], <[[NUM]] x [[TYPE]]> [[res9]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF10]], <[[NUM]] x [[TYPE]]> [[res10]], i32 [[ALN]])

  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF0]], <[[NUM]] x [[TYPE]]> [[res0]], i32 [[ALN]])
  // res1 is just vec0 since it was just the unary + operator.
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF1]], <[[NUM]] x [[TYPE]]> [[vec0]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF2]], <[[NUM]] x [[TYPE]]> [[res2]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF3]], <[[NUM]] x [[TYPE]]> [[res3]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF4]], <[[NUM]] x [[TYPE]]> [[res4]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF5]], <[[NUM]] x [[TYPE]]> [[res5]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF6]], <[[NUM]] x [[TYPE]]> [[res6]], i32 [[ALN]])
  // res[] input gets either the original or the preincremented value.
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF7]], <[[NUM]] x [[TYPE]]> [[vec7]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF8]], <[[NUM]] x [[TYPE]]> [[vec8]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF9]], <[[NUM]] x [[TYPE]]> [[res9]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF10]], <[[NUM]] x [[TYPE]]> [[res10]], i32 [[ALN]])

  return res;
}

// Test arithmetic operators with scalars.
vector<TYPE, NUM> scarithmetic(vector<TYPE, NUM> things[11], TYPE scales[10])[11] {
  vector<TYPE, NUM> res[11];

  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 3
  // CHECK: [[ResHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Output]]
  // CHECK: [[VecIx:%.*]] = add i32 [[InIx1]], 3
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF0]], i32 [[ALN]])
  // CHECK: [[vec0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF1]], i32 [[ALN]])
  // CHECK: [[vec1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF2]], i32 [[ALN]])
  // CHECK: [[vec2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF3]], i32 [[ALN]])
  // CHECK: [[vec3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF4]], i32 [[ALN]])
  // CHECK: [[vec4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF5]], i32 [[ALN]])
  // CHECK: [[vec5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF6]], i32 [[ALN]])
  // CHECK: [[vec6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // CHECK: [[SclIx:%.*]] = add i32 [[InIx2]], 3
  // CHECK: [[SclHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Scales]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[OFF0]], i8 1, i32 [[ALN]])
  // CHECK: [[scl0:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[SOFF1]], i8 1, i32 [[ALN]])
  // CHECK: [[scl1:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[SOFF2]], i8 1, i32 [[ALN]])
  // CHECK: [[scl2:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[SOFF3]], i8 1, i32 [[ALN]])
  // CHECK: [[scl3:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[SOFF4]], i8 1, i32 [[ALN]])
  // CHECK: [[scl4:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[SOFF5]], i8 1, i32 [[ALN]])
  // CHECK: [[scl5:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferLoad.[[STY]](i32 139, %dx.types.Handle [[SclHdl]], i32 [[SclIx]], i32 [[SOFF6]], i8 1, i32 [[ALN]])
  // CHECK: [[scl6:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl0]], i32 0
  // CHECK: [[spt0:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[spt0]], [[vec0]]
  res[0] = things[0] + scales[0];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl1]], i32 0
  // CHECK: [[spt1:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res1:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[vec1]], [[spt1]]
  res[1] = things[1] - scales[1];


  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl2]], i32 0
  // CHECK: [[spt2:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res2:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[spt2]], [[vec2]]
  res[2] = things[2] * scales[2];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl3]], i32 0
  // CHECK: [[spt3:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res3:%[0-9]*]] = [[DIV]] <[[NUM]] x [[TYPE]]> [[vec3]], [[spt3]]
  res[3] = things[3] / scales[3];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl4]], i32 0
  // CHECK: [[spt4:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res4:%[0-9]*]] = [[ADD]] <[[NUM]] x [[TYPE]]> [[spt4]], [[vec4]]
  res[4] = scales[4] + things[4];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl5]], i32 0
  // CHECK: [[spt5:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res5:%[0-9]*]] = [[SUB]] <[[NUM]] x [[TYPE]]> [[spt5]], [[vec5]]
  res[5] = scales[5] - things[5];

  // CHECK: [[spt:%[0-9]*]] = insertelement <[[NUM]] x [[TYPE]]> undef, [[TYPE]] [[scl6]], i32 0
  // CHECK: [[spt6:%[0-9]*]] = shufflevector <[[NUM]] x [[TYPE]]> [[spt]], <[[NUM]] x [[TYPE]]> undef, <[[NUM]] x i32> zeroinitializer
  // CHECK: [[res6:%[0-9]*]] = [[MUL]] <[[NUM]] x [[TYPE]]> [[spt6]], [[vec6]]
  res[6] = scales[6] * things[6];
  res[7] = res[8] = res[9] = res[10] = 0;

  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF0]], <[[NUM]] x [[TYPE]]> [[res0]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF1]], <[[NUM]] x [[TYPE]]> [[res1]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF2]], <[[NUM]] x [[TYPE]]> [[res2]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF3]], <[[NUM]] x [[TYPE]]> [[res3]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF4]], <[[NUM]] x [[TYPE]]> [[res4]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF5]], <[[NUM]] x [[TYPE]]> [[res5]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF6]], <[[NUM]] x [[TYPE]]> [[res6]], i32 [[ALN]])

  return res;
}

// Test logic operators.
// Only permissable in pre-HLSL2021
vector<bool, NUM> logic(vector<bool, NUM> truth[10], vector<TYPE, NUM> consequences[11])[10] {
  vector<bool, NUM> res[10];
  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 4
  // CHECK: [[TruHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Truths]]
  // CHECK: [[TruIx:%.*]] = add i32 [[InIx2]], 4
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF0]], i32 [[IALN]])
  // CHECK: [[ivec0:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF1]], i32 [[IALN]])
  // CHECK: [[ivec1:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF2]], i32 [[IALN]])
  // CHECK: [[ivec2:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF3]], i32 [[IALN]])
  // CHECK: [[ivec3:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF4]], i32 [[IALN]])
  // CHECK: [[ivec4:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[TruHdl]], i32 [[TruIx]], i32 [[BOFF5]], i32 [[IALN]])
  // CHECK: [[ivec5:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0

  // CHECK: [[VecIx:%.*]] = add i32 [[InIx1]], 4
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF0]], i32 [[ALN]])
  // CHECK: [[vec0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF1]], i32 [[ALN]])
  // CHECK: [[vec1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF2]], i32 [[ALN]])
  // CHECK: [[vec2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF3]], i32 [[ALN]])
  // CHECK: [[vec3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF4]], i32 [[ALN]])
  // CHECK: [[vec4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF5]], i32 [[ALN]])
  // CHECK: [[vec5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF6]], i32 [[ALN]])
  // CHECK: [[vec6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0


  // CHECK: [[cmp:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[ivec0]], zeroinitializer
  // CHECK: [[cmp0:%[0-9]*]] = icmp eq <[[NUM]] x i1> [[cmp]], zeroinitializer
  // CHECK: [[res0:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp0]] to <[[NUM]] x i32>
  res[0] = !truth[0];

  // CHECK: [[bvec1:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[ivec1]], zeroinitializer
  // CHECK: [[bvec2:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[ivec2]], zeroinitializer
  // CHECK: [[bres1:%[0-9]*]] = or <[[NUM]] x i1> [[bvec2]], [[bvec1]]
  // CHECK: [[res1:%[0-9]*]] = zext <[[NUM]] x i1> [[bres1]] to <[[NUM]] x i32>
  res[1] = truth[1] || truth[2];

  // CHECK: [[bvec3:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[ivec3]], zeroinitializer
  // CHECK: [[bres2:%[0-9]*]] = and <[[NUM]] x i1> [[bvec3]], [[bvec2]]
  // CHECK: [[res2:%[0-9]*]] = zext <[[NUM]] x i1> [[bres2]] to <[[NUM]] x i32>
  res[2] = truth[2] && truth[3];

  // CHECK: [[bvec4:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[ivec4]], zeroinitializer
  // CHECK: [[bvec5:%[0-9]*]] = icmp ne <[[NUM]] x i32> [[ivec5]], zeroinitializer
  // CHECK: [[bres3:%[0-9]*]] = select <[[NUM]] x i1> [[bvec3]], <[[NUM]] x i1> [[bvec4]], <[[NUM]] x i1> [[bvec5]]
  // CHECK: [[res3:%[0-9]*]] = zext <[[NUM]] x i1> [[bres3]] to <[[NUM]] x i32>
  res[3] = truth[3] ? truth[4] : truth[5];

  // CHECK: [[cmp4:%[0-9]*]] = [[CMP:[fi]?cmp( fast)?]] {{o?}}eq <[[NUM]] x [[TYPE]]> [[vec0]], [[vec1]]
  // CHECK: [[res4:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp4]] to <[[NUM]] x i32>
  res[4] = consequences[0] == consequences[1];

  // CHECK: [[cmp5:%[0-9]*]] = [[CMP]] {{u?}}ne <[[NUM]] x [[TYPE]]> [[vec1]], [[vec2]]
  // CHECK: [[res5:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp5]] to <[[NUM]] x i32>
  res[5] = consequences[1] != consequences[2];

  // CHECK: [[cmp6:%[0-9]*]] = [[CMP]] {{[osu]?}}lt <[[NUM]] x [[TYPE]]> [[vec2]], [[vec3]]
  // CHECK: [[res6:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp6]] to <[[NUM]] x i32>
  res[6] = consequences[2] <  consequences[3];

  // CHECK: [[cmp7:%[0-9]*]] = [[CMP]] {{[osu]]?}}gt <[[NUM]] x [[TYPE]]> [[vec3]], [[vec4]]
  // CHECK: [[res7:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp7]] to <[[NUM]] x i32>
  res[7] = consequences[3] >  consequences[4];

  // CHECK: [[cmp8:%[0-9]*]] = [[CMP]] {{[osu]]?}}le <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  // CHECK: [[res8:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp8]] to <[[NUM]] x i32>
  res[8] = consequences[4] <= consequences[5];

  // CHECK: [[cmp9:%[0-9]*]] = [[CMP]] {{[osu]?}}ge <[[NUM]] x [[TYPE]]> [[vec5]], [[vec6]]
  // CHECK: [[res9:%[0-9]*]] = zext <[[NUM]] x i1> [[cmp9]] to <[[NUM]] x i32>
  res[9] = consequences[5] >= consequences[6];

  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF0]], <[[NUM]] x i32> [[res0]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF1]], <[[NUM]] x i32> [[res1]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF2]], <[[NUM]] x i32> [[res2]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF3]], <[[NUM]] x i32> [[res3]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF4]], <[[NUM]] x i32> [[res4]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF5]], <[[NUM]] x i32> [[res5]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF6]], <[[NUM]] x i32> [[res6]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF7]], <[[NUM]] x i32> [[res7]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF8]], <[[NUM]] x i32> [[res8]], i32 4)
  // CHECK: call void @dx.op.rawBufferVectorStore.[[ITY]](i32 304, %dx.types.Handle [[TruHdl]], i32 [[ResIx]], i32 [[BOFF9]], <[[NUM]] x i32> [[res9]], i32 4)

  return res;
}

static const int Ix = 2;

// Test indexing operators
vector<TYPE, NUM> index(vector<TYPE, NUM> things[11], int i)[11] {
  vector<TYPE, NUM> res[11];

  // CHECK: [[ResIx:%.*]] = add i32 [[OutIx]], 5
  // CHECK: [[ResHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Output]]
  // CHECK: [[VecIx:%.*]] = add i32 [[InIx1]], 5
  // CHECK: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Input]]

  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF0]], i32 [[ALN]])
  // CHECK: [[vec0:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec0]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 1
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF1]], i32 [[ALN]])
  // CHECK: [[vec1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec1]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 2
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF2]], i32 [[ALN]])
  // CHECK: [[vec2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec2]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 3
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF3]], i32 [[ALN]])
  // CHECK: [[vec3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec3]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 4
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF4]], i32 [[ALN]])
  // CHECK: [[vec4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec4]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 5
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF5]], i32 [[ALN]])
  // CHECK: [[vec5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec5]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 6
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF6]], i32 [[ALN]])
  // CHECK: [[vec6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec6]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 7
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF7]], i32 [[ALN]])
  // CHECK: [[vec7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec7]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 8
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF8]], i32 [[ALN]])
  // CHECK: [[vec8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec8]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 9
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF9]], i32 [[ALN]])
  // CHECK: [[vec9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec9]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 10
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VecIx]], i32 [[OFF10]], i32 [[ALN]])
  // CHECK: [[vec10:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec10]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]

  // CHECK: [[Ix:%.*]] = add i32 [[InIx2]], 5

  // CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 0
  // CHECK: store <[[NUM]] x [[TYPE]]> zeroinitializer, <[[NUM]] x [[TYPE]]>* [[adr0]], align [[ALN]]
  res[0] = 0;


  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 [[Ix]]
  // CHECK: store <[[NUM]] x [[TYPE]]> <[[TYPE]] [[POS1]],{{[^>]*}}>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  res[i] = 1;

  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 2
  // CHECK: store <[[NUM]] x [[TYPE]]> <[[TYPE]] [[TWO:(2|2\.?0*e?\+?0*|0xH4000)]],{{[^>]*}}>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  res[Ix] = 2;

  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 3
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec0]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  res[3] = things[0];

  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch2]], i32 0, i32 [[Ix]]
  // CHECK: [[ldix:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 4
  // CHECK: store <[[NUM]] x [[TYPE]]> [[ldix]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  res[4] = things[i];


  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 5
  // CHECK: store <[[NUM]] x [[TYPE]]> [[vec2]], <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  res[5] = things[Ix];

  // CHECK: [[ld:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr0]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 0, <[[NUM]] x [[TYPE]]> [[ld]], i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 1
  // CHECK: [[ld:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF1]], <[[NUM]] x [[TYPE]]> [[ld]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF2]], <[[NUM]] x [[TYPE]]> <[[TYPE]] [[TWO]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF3]], <[[NUM]] x [[TYPE]]> [[vec0]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF4]], <[[NUM]] x [[TYPE]]> [[ldix]], i32 [[ALN]])
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF5]], <[[NUM]] x [[TYPE]]> [[vec2]], i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 6
  // CHECK: [[ld:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF6]], <[[NUM]] x [[TYPE]]> [[ld]], i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 7
  // CHECK: [[ld:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF7]], <[[NUM]] x [[TYPE]]> [[ld]], i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 8
  // CHECK: [[ld:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF8]], <[[NUM]] x [[TYPE]]> [[ld]], i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 9
  // CHECK: [[ld:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF9]], <[[NUM]] x [[TYPE]]> [[ld]], i32 [[ALN]])
  // CHECK: [[adr:%.*]] = getelementptr inbounds [11 x <[[NUM]] x [[TYPE]]>], [11 x <[[NUM]] x [[TYPE]]>]* [[scratch1]], i32 0, i32 10
  // CHECK: [[ld:%.*]] = load <[[NUM]] x [[TYPE]]>, <[[NUM]] x [[TYPE]]>* [[adr]], align [[ALN]]
  // CHECK: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[ResHdl]], i32 [[ResIx]], i32 [[OFF10]], <[[NUM]] x [[TYPE]]> [[ld]], i32 [[ALN]])

  return res;
}

#ifdef INT
// Test bit twiddling operators.
void bittwiddlers(inout vector<TYPE, NUM> things[13]) {
  // INT: [[VcIx:%.*]] = add i32 [[InIx1]], 6
  // INT: [[InHdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[Bits]]
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF1]], i32 [[ALN]])
  // INT: [[vec1:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF2]], i32 [[ALN]])
  // INT: [[vec2:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF3]], i32 [[ALN]])
  // INT: [[vec3:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF4]], i32 [[ALN]])
  // INT: [[vec4:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF5]], i32 [[ALN]])
  // INT: [[vec5:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF6]], i32 [[ALN]])
  // INT: [[vec6:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF7]], i32 [[ALN]])
  // INT: [[vec7:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF8]], i32 [[ALN]])
  // INT: [[vec8:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF9]], i32 [[ALN]])
  // INT: [[vec9:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF10]], i32 [[ALN]])
  // INT: [[vec10:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF11]], i32 [[ALN]])
  // INT: [[vec11:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0
  // INT: [[ld:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferVectorLoad.[[TY]](i32 303, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF12]], i32 [[ALN]])
  // INT: [[vec12:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[ld]], 0

  // INT: [[res0:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[vec1]], <[[TYPE]] -1
  things[0] = ~things[1];

  // INT: [[res1:%[0-9]*]] = or <[[NUM]] x [[TYPE]]> [[vec3]], [[vec2]]
  things[1] = things[2] | things[3];

  // INT: [[res2:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec4]], [[vec3]]
  things[2] = things[3] & things[4];

  // INT: [[res3:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[vec4]], [[vec5]]
  things[3] = things[4] ^ things[5];

  // INT: [[shv6:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec6]]
  // INT: [[res4:%[0-9]*]] = shl <[[NUM]] x [[TYPE]]> [[vec5]], [[shv6]]
  things[4] = things[5] << things[6];

  // INT: [[shv7:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec7]]
  // UNSIG: [[res5:%[0-9]*]] = lshr <[[NUM]] x [[TYPE]]> [[vec6]], [[shv7]]
  // SIG: [[res5:%[0-9]*]] = ashr <[[NUM]] x [[TYPE]]> [[vec6]], [[shv7]]
  things[5] = things[6] >> things[7];

  // INT: [[res6:%[0-9]*]] = or <[[NUM]] x [[TYPE]]> [[vec8]], [[vec6]]
  things[6] |= things[8];

  // INT: [[res7:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec9]], [[vec7]]
  things[7] &= things[9];

  // INT: [[res8:%[0-9]*]] = xor <[[NUM]] x [[TYPE]]> [[vec8]], [[vec10]]
  things[8] ^= things[10];

  // INT: [[shv11:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec11]]
  // INT: [[res9:%[0-9]*]] = shl <[[NUM]] x [[TYPE]]> [[vec9]], [[shv11]]
  things[9] <<= things[11];

  // INT: [[shv12:%[0-9]*]] = and <[[NUM]] x [[TYPE]]> [[vec12]]
  // UNSIG: [[res10:%[0-9]*]] = lshr <[[NUM]] x [[TYPE]]> [[vec10]], [[shv12]]
  // SIG: [[res10:%[0-9]*]] = ashr <[[NUM]] x [[TYPE]]> [[vec10]], [[shv12]]
  things[10] >>= things[12];

  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF0]], <[[NUM]] x [[TYPE]]> [[res0]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF1]], <[[NUM]] x [[TYPE]]> [[res1]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF2]], <[[NUM]] x [[TYPE]]> [[res2]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF3]], <[[NUM]] x [[TYPE]]> [[res3]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF4]], <[[NUM]] x [[TYPE]]> [[res4]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF5]], <[[NUM]] x [[TYPE]]> [[res5]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF6]], <[[NUM]] x [[TYPE]]> [[res6]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF7]], <[[NUM]] x [[TYPE]]> [[res7]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF8]], <[[NUM]] x [[TYPE]]> [[res8]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF9]], <[[NUM]] x [[TYPE]]> [[res9]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF10]], <[[NUM]] x [[TYPE]]> [[res10]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF11]], <[[NUM]] x [[TYPE]]> [[vec11]], i32 [[ALN]])
  // INT: call void @dx.op.rawBufferVectorStore.[[TY]](i32 304, %dx.types.Handle [[InHdl]], i32 [[VcIx]], i32 [[OFF12]], <[[NUM]] x [[TYPE]]> [[vec12]], i32 [[ALN]])

  // CHECK-LABEL: ret void
}
#endif // INT
