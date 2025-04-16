// RUN: %dxc -DTYPE=float    -T vs_6_6 %s | FileCheck %s
// RUN: %dxc -DTYPE=bool     -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,I1
// RUN: %dxc -DTYPE=uint64_t -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,I64
// RUN: %dxc -DTYPE=double   -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,F64

// RUN: %dxc -DTYPE=float1    -T vs_6_6 %s | FileCheck %s
// RUN: %dxc -DTYPE=bool1     -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,I1
// RUN: %dxc -DTYPE=uint64_t1 -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,I64
// RUN: %dxc -DTYPE=double1   -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,F64

// Confirm that 6.9 doesn't use vector loads for scalars and vec1s
// RUN: %dxc -DTYPE=float    -T vs_6_9 %s | FileCheck %s
// RUN: %dxc -DTYPE=bool     -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,I1
// RUN: %dxc -DTYPE=uint64_t -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,I64
// RUN: %dxc -DTYPE=double   -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,F64

// RUN: %dxc -DTYPE=float1    -T vs_6_9 %s | FileCheck %s
// RUiN: %dxc -DTYPE=bool1     -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,I1
// RUN: %dxc -DTYPE=uint64_t1 -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,I64
// RUN: %dxc -DTYPE=double1   -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,F64

///////////////////////////////////////////////////////////////////////
// Test codegen for various load and store operations and conversions
//  for different scalar buffer types and confirm that the proper
//  loads, stores, and conversion operations take place.
///////////////////////////////////////////////////////////////////////


// These -DAGs must match the same line. That is the only reason for the -DAG.
// The first match will assign [[TY]] to the native type
// For most runs, the second match will assign [[TY32]] to the same thing.
// For 64-bit types, the memory representation is i32 and a separate variable is needed.
// For these cases, there is another line that will always match i32.
// This line will also force the previous -DAGs to match the same line since the most
// This shader can produce is two ResRet types.
// CHECK-DAG: %dx.types.ResRet.[[TY:[a-z][0-9][0-9]]] = type { [[TYPE:[a-z0-9]*]],
// CHECK-DAG: %dx.types.ResRet.[[TY32:[a-z][0-9][0-9]]] = type { [[TYPE]],
// I64: %dx.types.ResRet.[[TY32:i32]]
// F64: %dx.types.ResRet.[[TY32:i32]]

  ByteAddressBuffer RoByBuf : register(t1);
RWByteAddressBuffer RwByBuf : register(u1);

  StructuredBuffer< TYPE > RoStBuf : register(t2);
RWStructuredBuffer< TYPE > RwStBuf : register(u2);

  Buffer< TYPE > RoTyBuf : register(t3);
RWBuffer< TYPE > RwTyBuf : register(u3);

ConsumeStructuredBuffer<TYPE> CnStBuf : register(u4);
AppendStructuredBuffer<TYPE> ApStBuf  : register(u5);

void main(uint ix[2] : IX) {
  // ByteAddressBuffer Tests

  // CHECK-DAG: [[HDLROBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)
  // CHECK-DAG: [[HDLRWBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 1 }, i32 1, i1 false)

  // CHECK-DAG: [[HDLROST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)
  // CHECK-DAG: [[HDLRWST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 1 }, i32 2, i1 false)

  // CHECK-DAG: [[HDLROTY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 0 }, i32 3, i1 false)
  // CHECK-DAG: [[HDLRWTY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 1 }, i32 3, i1 false)

  // CHECK-DAG: [[HDLCON:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 4, i32 4, i32 0, i8 1 }, i32 4, i1 false)
  // CHECK-DAG: [[HDLAPP:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 5, i32 5, i32 0, i8 1 }, i32 5, i1 false)

  // CHECK: [[IX0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4,

  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE babElt1 = RwByBuf.Load< TYPE >(ix[0]);

  // CHECK: [[ANHDLROBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROBY]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE babElt2 = RoByBuf.Load< TYPE >(ix[0]);

  // I1: zext i1 %{{.*}} to i32
  // CHECK: all void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0]]
  RwByBuf.Store< TYPE >(ix[0], babElt1 + babElt2);

  // StructuredBuffer Tests
  // CHECK: [[ANHDLRWST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWST]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt1 = RwStBuf.Load(ix[0]);
  // CHECK: [[IX1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4,
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX1]]
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt2 = RwStBuf[ix[1]];

  // CHECK: [[ANHDLROST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROST]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt3 = RoStBuf.Load(ix[0]);
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX1]]
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt4 = RoStBuf[ix[1]];

  // I1: zext i1 %{{.*}} to i32
  // CHECK: all void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]]
  RwStBuf[ix[0]] = stbElt1 + stbElt2 + stbElt3 + stbElt4;

  // {Append/Consume}StructuredBuffer Tests
  // CHECK: [[ANHDLCON:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLCON]]
  // CHECK: [[CONIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLCON]], i8 -1)
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]]
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE cnElt = CnStBuf.Consume();

  // CHECK: [[ANHDLAPP:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLAPP]]
  // CHECK: [[APPIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLAPP]], i8 1)
  // I1: zext i1 %{{.*}} to i32
  // CHECK: all void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]]
  ApStBuf.Append(cnElt);

  // TypedBuffer Tests
  // CHECK: [[ANHDLRWTY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWTY]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLRWTY]], i32 [[IX0]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt1 = RwTyBuf.Load(ix[0]);
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLRWTY]], i32 [[IX1]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt2 = RwTyBuf[ix[1]];
  // CHECK: [[ANHDLROTY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROTY]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLROTY]], i32 [[IX0]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt3 = RoTyBuf.Load(ix[0]);
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLROTY]], i32 [[IX1]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt4 = RoTyBuf[ix[1]];

  // F64: call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102
  // I64: trunc i64 %{{.*}} to i32
  // I64: lshr i64  %{{.*}}, 32
  // I64: trunc i64 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // CHECK: all void @dx.op.bufferStore.[[TY32]](i32 69, %dx.types.Handle [[ANHDLRWTY]], i32 [[IX0]]
  RwTyBuf[ix[0]] = typElt1 + typElt2 + typElt3 + typElt4;
}
