// RUN: %dxc -DTYPE=float4    -T vs_6_6 %s | FileCheck %s
// RUN: %dxc -DTYPE=bool4     -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,I1
// RUN: %dxc -DTYPE=uint64_t2 -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,I64
// RUN: %dxc -DTYPE=double2   -T vs_6_6 %s | FileCheck %s --check-prefixes=CHECK,F64

///////////////////////////////////////////////////////////////////////
// Test codegen for various load and store operations and conversions
//  for different scalar/vector buffer types and indices.
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

ConsumeStructuredBuffer<TYPE> CnStBuf : register(u3);
AppendStructuredBuffer<TYPE> ApStBuf  : register(u4);

  Buffer< TYPE > RoTyBuf : register(t5);
RWBuffer< TYPE > RwTyBuf : register(u5);

  Texture1D< TYPE > RoTex1d : register(t6);
RWTexture1D< TYPE > RwTex1d : register(u6);
  Texture2D< TYPE > RoTex2d : register(t7);
RWTexture2D< TYPE > RwTex2d : register(u7);
  Texture3D< TYPE > RoTex3d : register(t8);
RWTexture3D< TYPE > RwTex3d : register(u8);

void main(uint ix0 : IX0, uint ix1 : IX1, uint2 ix2 : IX2, uint3 ix3 : IX3) {
  // ByteAddressBuffer Tests

  // CHECK-DAG: [[HDLROBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)
  // CHECK-DAG: [[HDLRWBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 1 }, i32 1, i1 false)

  // CHECK-DAG: [[HDLROST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)
  // CHECK-DAG: [[HDLRWST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 1 }, i32 2, i1 false)

  // CHECK-DAG: [[HDLCON:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 1 }, i32 3, i1 false)
  // CHECK-DAG: [[HDLAPP:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 4, i32 4, i32 0, i8 1 }, i32 4, i1 false)

  // CHECK-DAG: [[HDLROTY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 5, i32 5, i32 0, i8 0 }, i32 5, i1 false)
  // CHECK-DAG: [[HDLRWTY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 5, i32 5, i32 0, i8 1 }, i32 5, i1 false)

  // CHECK-DAG: [[HDLROTX1:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 6, i32 6, i32 0, i8 0 }, i32 6, i1 false)
  // CHECK-DAG: [[HDLRWTX1:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 6, i32 6, i32 0, i8 1 }, i32 6, i1 false)
  // CHECK-DAG: [[HDLROTX2:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 7, i32 7, i32 0, i8 0 }, i32 7, i1 false)
  // CHECK-DAG: [[HDLRWTX2:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 7, i32 7, i32 0, i8 1 }, i32 7, i1 false)
  // CHECK-DAG: [[HDLROTX3:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 8, i32 8, i32 0, i8 0 }, i32 8, i1 false)
  // CHECK-DAG: [[HDLRWTX3:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 8, i32 8, i32 0, i8 1 }, i32 8, i1 false)


  // CHECK-DAG: [[IX0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0
  // CHECK-DAG: [[IX1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0
  // CHECK-DAG: [[IX20:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0
  // CHECK-DAG: [[IX21:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 1
  // CHECK-DAG: [[IX30:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 0
  // CHECK-DAG: [[IX31:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 1
  // CHECK-DAG: [[IX32:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 2

  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE babElt1 = RwByBuf.Load< TYPE >(ix0);

  // CHECK: [[ANHDLROBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROBY]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE babElt2 = RoByBuf.Load< TYPE >(ix0);

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX1]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK1:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  uint status1 = 0;
  TYPE babElt3 = RwByBuf.Load< TYPE >(ix1, status1);

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROBY]], i32 [[IX1]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK2:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  uint status2 = 0;
  TYPE babElt4 = RoByBuf.Load< TYPE >(ix1, status2);

  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // CHECK: all void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0]]
  // CHECK: and i1 [[CHK1]], [[CHK2]]
  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 100
  RwByBuf.Store< TYPE >(ix0, babElt1 + babElt2 + babElt3 + babElt4);
  RwByBuf.Store< uint > (100, status1 && status2);

  // StructuredBuffer Tests
  // CHECK: [[ANHDLRWST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWST]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt1 = RwStBuf.Load(ix0);

  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX1]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt2 = RwStBuf[ix1];

  // CHECK: [[ANHDLROST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROST]]
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX0]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt3 = RoStBuf.Load(ix0);

  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX1]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt4 = RoStBuf[ix1];

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLRWST]], i32 [[IX20]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK1:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt5 = RwStBuf.Load(ix2[0], status1);

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLROST]], i32 [[IX20]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY]] [[RESRET]], 4
  // CHECK: [[CHK2:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE stbElt6 = RoStBuf.Load(ix2[0], status2);

  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // CHECK: all void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]]
  // CHECK: and i1 [[CHK1]], [[CHK2]]
  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 200
  RwStBuf[ix0] = stbElt1 + stbElt2 + stbElt3 + stbElt4 + stbElt5 + stbElt6;
  RwByBuf.Store< uint > (200, status1 && status2);

  // {Append/Consume}StructuredBuffer Tests
  // CHECK: [[ANHDLCON:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLCON]]
  // CHECK: [[CONIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLCON]], i8 -1) 
  // CHECK: call %dx.types.ResRet.[[TY]] @dx.op.rawBufferLoad.[[TY]](i32 139, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]]
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE cnElt = CnStBuf.Consume();

  // CHECK: [[ANHDLAPP:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLAPP]]
  // CHECK: [[APPIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLAPP]], i8 1) 
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // CHECK: all void @dx.op.rawBufferStore.[[TY]](i32 140, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]]
  ApStBuf.Append(cnElt);

  // TypedBuffer Tests
  // CHECK: [[ANHDLRWTY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWTY]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLRWTY]], i32 [[IX0]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt1 = RwTyBuf.Load(ix0);

  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLRWTY]], i32 [[IX1]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt2 = RwTyBuf[ix1];

  // CHECK: [[ANHDLROTY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROTY]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLROTY]], i32 [[IX0]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt3 = RoTyBuf.Load(ix0);

  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLROTY]], i32 [[IX1]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt4 = RoTyBuf[ix1];

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLRWTY]], i32 [[IX20]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY32]] [[RESRET]], 4
  // CHECK: [[CHK1:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt5 = RwTyBuf.Load(ix2[0], status1);

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[TY32]] @dx.op.bufferLoad.[[TY32]](i32 68, %dx.types.Handle [[ANHDLROTY]], i32 [[IX20]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[TY32]] [[RESRET]], 4
  // CHECK: [[CHK2:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE typElt6 = RoTyBuf.Load(ix2[0], status2);

  // F64: call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102
  // F64: call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102
  // I64: trunc i64 %{{.*}} to i32
  // lshr i64  %{{.*}}, 32
  // I64: trunc i64 %{{.*}} to i32
  // I64: trunc i64 %{{.*}} to i32
  // lshr i64  %{{.*}}, 32
  // I64: trunc i64 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // CHECK: call void @dx.op.bufferStore.[[TY32]](i32 69, %dx.types.Handle [[ANHDLRWTY]], i32 [[IX0]]
  // CHECK: and i1 [[CHK1]], [[CHK2]]
  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 300
  RwTyBuf[ix0] = typElt1 + typElt2 + typElt3 + typElt4 + typElt5 + typElt6;
  RwByBuf.Store< uint > (300, status1 && status2);

  // Texture Tests
  // CHECK: [[ANHDLROTX1:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROTX1]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.textureLoad.[[TY32]](i32 66, %dx.types.Handle [[ANHDLROTX1]], i32 0, i32 [[IX0]], i32 undef, i32 undef
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE texElt1 = RoTex1d[ix0];

  // CHECK: [[ANHDLRWTX1:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWTX1]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.textureLoad.[[TY32]](i32 66, %dx.types.Handle [[ANHDLRWTX1]], i32 undef, i32 [[IX0]], i32 undef, i32 undef
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE texElt2 = RwTex1d[ix0];

  // CHECK: [[ANHDLROTX2:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROTX2]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.textureLoad.[[TY32]](i32 66, %dx.types.Handle [[ANHDLROTX2]], i32 0, i32 [[IX20]], i32 [[IX21]], i32 undef
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE texElt3 = RoTex2d[ix2];

  // CHECK: [[ANHDLRWTX2:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWTX2]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.textureLoad.[[TY32]](i32 66, %dx.types.Handle [[ANHDLRWTX2]], i32 undef, i32 [[IX20]], i32 [[IX21]], i32 undef
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE texElt4 = RwTex2d[ix2];

  // CHECK: [[ANHDLROTX3:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROTX3]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.textureLoad.[[TY32]](i32 66, %dx.types.Handle [[ANHDLROTX3]], i32 0, i32 [[IX30]], i32 [[IX31]], i32 [[IX32]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE texElt5 = RoTex3d[ix3];

  // CHECK: [[ANHDLRWTX3:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWTX3]]
  // CHECK: call %dx.types.ResRet.[[TY32]] @dx.op.textureLoad.[[TY32]](i32 66, %dx.types.Handle [[ANHDLRWTX3]], i32 undef, i32 [[IX30]], i32 [[IX31]], i32 [[IX32]]
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // F64: call double @dx.op.makeDouble.f64(i32 101
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I64: zext i32 %{{.*}} to i64
  // I64: zext i32 %{{.*}} to i64
  // I64: shl nuw i64
  // I64: or i64
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  // I1: icmp ne i32 %{{.*}}, 0
  TYPE texElt6 = RwTex3d[ix3];

  // F64: call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102
  // F64: call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102
  // I64: trunc i64 %{{.*}} to i32
  // lshr i64  %{{.*}}, 32
  // I64: trunc i64 %{{.*}} to i32
  // I64: trunc i64 %{{.*}} to i32
  // lshr i64  %{{.*}}, 32
  // I64: trunc i64 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // I1: zext i1 %{{.*}} to i32
  // CHECK: call void @dx.op.textureStore.[[TY32]](i32 67, %dx.types.Handle [[ANHDLRWTX3]], i32 [[IX30]], i32 [[IX31]], i32 [[IX32]]
  RwTex3d[ix3] = texElt1 + texElt2 + texElt3 + texElt4 + texElt5 + texElt6;
}
