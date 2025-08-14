// RUN: %dxc -DTYPE=float    -DNUM=4 -T vs_6_9 %s | FileCheck %s
// RUN: %dxc -DTYPE=bool     -DNUM=4 -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,I1
// RUN: %dxc -DTYPE=uint64_t -DNUM=2 -T vs_6_9 %s | FileCheck %s
// RUN: %dxc -DTYPE=double   -DNUM=2 -T vs_6_9 %s | FileCheck %s

// RUN: %dxc -DTYPE=float    -DNUM=6  -T vs_6_9 %s | FileCheck %s
// RUN: %dxc -DTYPE=bool     -DNUM=13 -T vs_6_9 %s | FileCheck %s --check-prefixes=CHECK,I1
// RUN: %dxc -DTYPE=uint64_t -DNUM=24 -T vs_6_9 %s | FileCheck %s
// RUN: %dxc -DTYPE=double   -DNUM=32 -T vs_6_9 %s | FileCheck %s

///////////////////////////////////////////////////////////////////////
// Test codegen for various load and store operations and conversions
//  for different scalar/vector buffer types and indices.
///////////////////////////////////////////////////////////////////////

// CHECK: %dx.types.ResRet.[[VTY:v[0-9]*[a-z][0-9][0-9]]] = type { <[[NUM:[0-9]*]] x [[TYPE:[a-z_0-9]*]]>, i32 }

ByteAddressBuffer RoByBuf : register(t1);
RWByteAddressBuffer RwByBuf : register(u1);

StructuredBuffer<vector<TYPE, NUM> > RoStBuf : register(t2);
RWStructuredBuffer<vector<TYPE, NUM> > RwStBuf : register(u2);

ConsumeStructuredBuffer<vector<TYPE, NUM> > CnStBuf : register(u4);
AppendStructuredBuffer<vector<TYPE, NUM> > ApStBuf  : register(u5);

// CHECK-LABEL: define void @main
[shader("vertex")]
void main(uint ix[3] : IX) {
  // ByteAddressBuffer Tests

  // CHECK-DAG: [[HDLROBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)
  // CHECK-DAG: [[HDLRWBY:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 1 }, i32 1, i1 false)

  // CHECK-DAG: [[HDLROST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)
  // CHECK-DAG: [[HDLRWST:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 1 }, i32 2, i1 false)

  // CHECK-DAG: [[HDLCON:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 4, i32 4, i32 0, i8 1 }, i32 4, i1 false)
  // CHECK-DAG: [[HDLAPP:%.*]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 5, i32 5, i32 0, i8 1 }, i32 5, i1 false)

  // CHECK: [[IX0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4,

  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0]]
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  babElt1 = RwByBuf.Load< vector<TYPE, NUM>  >(ix[0]);

  // CHECK: [[IX1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4,
  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX1]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[VTY]] [[RESRET]], 1
  // CHECK: [[CHK1:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  uint status1;
  vector<TYPE, NUM>  babElt3 = RwByBuf.Load< vector<TYPE, NUM>  >(ix[1], status1);

  // CHECK: [[ANHDLROBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROBY]]
  // CHECK: call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLROBY]], i32 [[IX0]]
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  babElt2 = RoByBuf.Load< vector<TYPE, NUM>  >(ix[0]);

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLROBY]], i32 [[IX1]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[VTY]] [[RESRET]], 1
  // CHECK: [[CHK2:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  uint status2;
  vector<TYPE, NUM>  babElt4 = RoByBuf.Load< vector<TYPE, NUM>  >(ix[1], status2);

  // I1: zext <[[NUM]] x i1> %{{.*}} to <[[NUM]] x i32>
  // CHECK: all void @dx.op.rawBufferVectorStore.[[VTY]](i32 304, %dx.types.Handle [[ANHDLRWBY]], i32 [[IX0]]
  // CHECK: and i1 [[CHK1]], [[CHK2]]
  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 100
  RwByBuf.Store< vector<TYPE, NUM>  >(ix[0], babElt1 + babElt2 + babElt3 + babElt4);
  RwByBuf.Store< uint > (100, status1 && status2);

  // StructuredBuffer Tests
  // CHECK: [[ANHDLRWST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWST]]
  // CHECK: call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]]
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  stbElt1 = RwStBuf.Load(ix[0]);

  // CHECK: call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLRWST]], i32 [[IX1]]
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  stbElt2 = RwStBuf[ix[1]];

  // CHECK: [[IX2:%.*]] = call i32 @dx.op.loadInput.i32(i32 4,
  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLRWST]], i32 [[IX2]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[VTY]] [[RESRET]], 1
  // CHECK: [[CHK1:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  stbElt5 = RwStBuf.Load(ix[2], status1);

  // CHECK: [[ANHDLROST:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLROST]]
  // CHECK: call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLROST]], i32 [[IX0]]
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  stbElt3 = RoStBuf.Load(ix[0]);

  // CHECK: call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLROST]], i32 [[IX1]]
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  stbElt4 = RoStBuf[ix[1]];

  // CHECK: [[RESRET:%.*]] = call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLROST]], i32 [[IX2]]
  // CHECK: [[STATUS:%.*]] = extractvalue %dx.types.ResRet.[[VTY]] [[RESRET]], 1
  // CHECK: [[CHK2:%.*]] = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 [[STATUS]])
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  stbElt6 = RoStBuf.Load(ix[2], status2);

  // I1: zext <[[NUM]] x i1> %{{.*}} to <[[NUM]] x i32>
  // CHECK: all void @dx.op.rawBufferVectorStore.[[VTY]](i32 304, %dx.types.Handle [[ANHDLRWST]], i32 [[IX0]]
  // CHECK: and i1 [[CHK1]], [[CHK2]]
  // CHECK: [[ANHDLRWBY:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLRWBY]]
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[ANHDLRWBY]], i32 200
  RwStBuf[ix[0]] = stbElt1 + stbElt2 + stbElt3 + stbElt4 + stbElt5 + stbElt6;
  RwByBuf.Store< uint > (200, status1 && status2);

  // {Append/Consume}StructuredBuffer Tests
  // CHECK: [[ANHDLCON:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLCON]]
  // CHECK: [[CONIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLCON]], i8 -1) 
  // CHECK: call %dx.types.ResRet.[[VTY]] @dx.op.rawBufferVectorLoad.[[VTY]](i32 303, %dx.types.Handle [[ANHDLCON]], i32 [[CONIX]]
  // I1: icmp ne <[[NUM]] x i32> %{{.*}}, zeroinitializer
  vector<TYPE, NUM>  cnElt = CnStBuf.Consume();

  // CHECK: [[ANHDLAPP:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDLAPP]]
  // CHECK: [[APPIX:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[ANHDLAPP]], i8 1) 
  // I1: zext <[[NUM]] x i1> %{{.*}} to <[[NUM]] x i32>
  // CHECK: all void @dx.op.rawBufferVectorStore.[[VTY]](i32 304, %dx.types.Handle [[ANHDLAPP]], i32 [[APPIX]]
  ApStBuf.Append(cnElt);
}
