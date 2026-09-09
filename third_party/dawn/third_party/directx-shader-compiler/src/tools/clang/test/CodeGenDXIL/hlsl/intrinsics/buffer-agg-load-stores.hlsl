// RUN: %dxc -T vs_6_6              -DETY=float     -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=bool      -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=uint64_t  -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=double    -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI

// RUN: %dxc -T vs_6_6              -DETY=float1    -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=bool1     -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=uint64_t1 -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=double1   -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI

// RUN: %dxc -T vs_6_6              -DETY=float4    -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=bool4     -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=uint64_t4 -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6              -DETY=double4   -DCOLS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI

// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=float    -DCOLS=2 -DROWS=2 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=bool     -DCOLS=2 -DROWS=2 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=uint64_t -DCOLS=2 -DROWS=2 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=double   -DCOLS=2 -DROWS=2 %s | FileCheck %s

// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=float    -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=bool     -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=uint64_t -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6 -DATY=matrix -DETY=double   -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI

// RUN: %dxc -T vs_6_6 -DATY=Matrix -DETY=float    -DCOLS=2 -DROWS=2 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=Matrix -DETY=uint64_t -DCOLS=2 -DROWS=2 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=Matrix -DETY=double   -DCOLS=2 -DROWS=2 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=Matrix -DETY=float    -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6 -DATY=Matrix -DETY=bool     -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6 -DATY=Matrix -DETY=uint64_t -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI
// RUN: %dxc -T vs_6_6 -DATY=Matrix -DETY=double   -DCOLS=3 -DROWS=3 %s | FileCheck %s --check-prefixes=CHECK,MULTI

// RUN: %dxc -T vs_6_6 -DATY=Vector -DETY=float    -DCOLS=4 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=Vector -DETY=bool     -DCOLS=4 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=Vector -DETY=uint64_t -DCOLS=2 %s | FileCheck %s
// RUN: %dxc -T vs_6_6 -DATY=Vector -DETY=double   -DCOLS=2 %s | FileCheck %s

// RUN: %dxc -T vs_6_6 -DATY=OffVector -DETY=float    -DCOLS=4 %s | FileCheck %s --check-prefixes=CHECK,OFF
// RUN: %dxc -T vs_6_6 -DATY=OffVector -DETY=bool     -DCOLS=4 %s | FileCheck %s --check-prefixes=CHECK,OFF
// RUN: %dxc -T vs_6_6 -DATY=OffVector -DETY=uint64_t -DCOLS=2 %s | FileCheck %s --check-prefixes=CHECK,OFF
// RUN: %dxc -T vs_6_6 -DATY=OffVector -DETY=double   -DCOLS=2 %s | FileCheck %s --check-prefixes=CHECK,OFF

///////////////////////////////////////////////////////////////////////
// Test codegen for various load and store operations and conversions
//  for different aggregate buffer types and indices.
///////////////////////////////////////////////////////////////////////

// CHECK: %dx.types.ResRet.[[TY:[a-z][0-9][0-9]]] = type { [[TYPE:[a-z0-9]*]],

#if !defined(ATY)
// Arrays have no aggregate typename
#define TYPE ETY
#define SS [COLS]
#elif defined(ROWS)
// Matrices have two dimensions
#define TYPE ATY< ETY, COLS, ROWS>
#define SS
#else
// All else matches this formulation
#define TYPE ATY< ETY, COLS>
#define SS
#endif

template<typename T, int N>
struct Vector {
  vector<T, N> v;
  Vector operator+(Vector vec) {
    Vector ret;
    ret.v = v + vec.v;
    return ret;
  }
};

template<typename T, int N>
struct OffVector {
  float4 pad1;
  double pad2;
  vector<T, N> v;
  OffVector operator+(OffVector vec) {
    OffVector ret;
    ret.pad1 = 0.0;
    ret.pad2 = 0.0;
    ret.v = v + vec.v;
    return ret;
  }
};

template<typename T, int N, int M>
struct Matrix {
  matrix<T, N, M> m;
  Matrix operator+(Matrix mat) {
    Matrix ret;
    ret.m = m + mat.m;
    return ret;
  }
};

  ByteAddressBuffer RoByBuf : register(t1);
RWByteAddressBuffer RwByBuf : register(u1);

  StructuredBuffer< TYPE SS > RoStBuf : register(t2);
RWStructuredBuffer< TYPE SS > RwStBuf : register(u2);

ConsumeStructuredBuffer< TYPE SS > CnStBuf : register(u4);
AppendStructuredBuffer< TYPE SS > ApStBuf  : register(u5);

TYPE Add(TYPE f1[COLS], TYPE f2[COLS], TYPE f3[COLS], TYPE f4[COLS])[COLS] {
  TYPE ret[COLS];
  for (int i = 0; i < COLS; i++)
    ret[i] = f1[i] + f2[i] + f3[i] + f4[i];
  return ret;
}

template<typename T>
T Add(T v1, T v2, T v3, T v4) { return v1 + v2 + v3 + v4; }

TYPE Add(TYPE f1[COLS], TYPE f2[COLS], TYPE f3[COLS], TYPE f4[COLS], TYPE f5[COLS], TYPE f6[COLS])[COLS] {
  TYPE ret[COLS];
  for (int i = 0; i < COLS; i++)
    ret[i] = f1[i] + f2[i] + f3[i] + f4[i] + f5[i] + f6[i];
  return ret;
}

template<typename T>
T Add(T v1, T v2, T v3, T v4, T v5, T v6) { return v1 + v2 + v3 + v4 + v5 + v6; }

void main(uint ix[3] : IX) {
  // ByteAddressBuffer Tests

  // CHECK-DAG: [[HDLROBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)
  // CHECK-DAG: [[HDLRWBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 1 }, i32 1, i1 false)

  // CHECK-DAG: [[HDLROST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)
  // CHECK-DAG: [[HDLRWST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 1 }, i32 2, i1 false)

  // CHECK-DAG: [[HDLCON:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 4, i32 4, i32 0, i8 1 }, i32 4, i1 false)
  // CHECK-DAG: [[HDLAPP:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 5, i32 5, i32 0, i8 1 }, i32 5, i1 false)

  // These -DAGs must match the same line. That is the only reason for the -DAG.
  // The first match will assign [[IX0]] to the actual index value.
  // For most runs, the second match will assign [[RIX0]] to the same thing.
  // For ByteAddressBuffers (Raw Buffers), the index gets offset sometimes to account
  // for lack offset support and a separate variable is needed for this index + offset value.
  // For these cases, the OFF : lines below will match the updated index value with the new offsets.
  // These lines will always match the same line since this shader can only produce one loadInput call.
  // CHECK-DAG: [[IX0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 {{[0-9]*}}, i32 [[BOFF:0]]
  // CHECK-DAG: [[RIX0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 {{[0-9]*}}, i32 [[BOFF]]

  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // OFF: [[RIX0:%.*]] = add i32 [[IX0]], [[BOFF:[0-9]+]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[RIX0]]
  // MULTI: [[IX0p4:%.*]] = add i32 [[RIX0]], [[p4:[0-9]+]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0p4]]
  // MULTI: [[IX0p8:%.*]] = add i32 [[RIX0]], [[p8:[0-9]+]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0p8]]
  // I1: icmp ne i32
  // I1: icmp ne i32
  // I1: icmp ne i32
  // I1: icmp ne i32
  TYPE babElt1 SS = RwByBuf.Load< TYPE SS >(ix[0]);

  // CHECK-DAG: [[IX1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 {{[0-9]*}}, i32 1
  // CHECK-DAG: [[RIX1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 {{[0-9]*}}, i32 1
  // OFF: [[RIX1:%.*]] = add i32 [[IX1]], [[BOFF]]
  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[RIX1]]
  // MULTI: [[IX1p4:%.*]] = add i32 [[RIX1]], [[p4]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX1p4]]
  // MULTI: [[IX1p8:%.*]] = add i32 [[RIX1]], [[p8]]
  // MULTI: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX1p8]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK1:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32
  // I1: icmp ne i32
  // I1: icmp ne i32
  // I1: icmp ne i32
  uint status1;
  TYPE babElt3 SS = RwByBuf.Load< TYPE SS >(ix[1], status1);

  // CHECK: [[ANHDLROBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROBY]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[RIX0]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[IX0p4]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[IX0p8]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE babElt2 SS = RoByBuf.Load< TYPE SS >(ix[0]);

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[RIX1]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[IX1p4]]
  // MULTI: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[IX1p8]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK2:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  uint status2;
  TYPE babElt4 SS = RoByBuf.Load< TYPE SS >(ix[1], status2);

  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // OFF: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 {{%.*}}, i32 undef, float 0.0
  // OFF: call void @dx.op.rawBufferStore.f64(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 {{%.*}}, i32 undef, double 0.0
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 [[RIX0]]
  // MULTI: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0p4]]
  // MULTI: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0p8]]
  // CHECK: and i1 [[CHK1]], [[CHK2]]
  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 100
  RwByBuf.Store< TYPE SS >(ix[0], Add(babElt1, babElt2, babElt3, babElt4));
  RwByBuf.Store< uint > (100, status1 && status2);

  // StructuredBuffer Tests
  // CHECK: [[ANHDLRWST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWST]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 [[BOFF]]
  // MULTI:  call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 [[p4]]
  // MULTI:  call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 [[p8]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt1 SS = RwStBuf.Load(ix[0]);

  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX1]], i32 [[BOFF]]
  // MULTI:  call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX1]], i32 [[p4]]
  // MULTI:  call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX1]], i32 [[p8]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt2 SS = RwStBuf[ix[1]];

  // CHECK: [[IX2:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 {{[0-9]*}}, i32 2
  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX2]], i32 [[BOFF]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX2]], i32 [[p4]]
  // MULTI: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX2]], i32 [[p8]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK1:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt5 SS = RwStBuf.Load(ix[2], status1);

  // CHECK: [[ANHDLROST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROST]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX0]], i32 [[BOFF]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX0]], i32 [[p4]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX0]], i32 [[p8]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt3 SS = RoStBuf.Load(ix[0]);

  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX1]], i32 [[BOFF]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX1]], i32 [[p4]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX1]], i32 [[p8]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt4 SS = RoStBuf[ix[1]];

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX2]], i32 [[BOFF]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX2]], i32 [[p4]]
  // MULTI: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX2]], i32 [[p8]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK2:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt6 SS = RoStBuf.Load(ix[2], status2);

  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // OFF: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 0, float 0.0
  // OFF: call void @dx.op.rawBufferStore.f64(i32 140, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 16, double 0.0
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 [[BOFF]]
  // MULTI: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 [[p4]]
  // MULTI: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]], i32 [[p8]]
  // CHECK: and i1 [[CHK1]], [[CHK2]]
  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 200
  RwStBuf[ix[0]] = Add(stbElt1, stbElt2, stbElt3, stbElt4, stbElt5, stbElt6);
  RwByBuf.Store< uint > (200, status1 && status2);

  // {Append/Consume}StructuredBuffer Tests
  // CHECK: [[ANHDLCON:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLCON]]
  // CHECK: [[CONIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLCON]], i8 -1) 
  // OFF: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]], i32 
  // OFF: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]], i32 16
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]], i32 [[BOFF]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]], i32 [[p4]]
  // MULTI: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]], i32 [[p8]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE cnElt SS = CnStBuf.Consume();

  // CHECK: [[ANHDLAPP:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLAPP]]
  // CHECK: [[APPIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLAPP]], i8 1) 
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // OFF: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]], i32 0
  // OFF: call void @dx.op.rawBufferStore.f64(i32 140, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]], i32 16
  // CHECK: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]], i32 [[BOFF]]
  // MULTI: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]], i32 [[p4]]
  // MULTI: call void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]], i32 [[p8]]
  ApStBuf.Append(cnElt);
}
