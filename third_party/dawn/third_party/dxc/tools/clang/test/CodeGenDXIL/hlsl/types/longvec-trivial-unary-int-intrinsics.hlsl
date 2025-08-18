// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=reversebits   -DOP=30 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=reversebits   -DOP=30 -DNUM=1022 %s | FileCheck %s

// Test vector-enabled unary intrinsics that take signed and unsigned integer parameters of
// different widths and are "trivial" in that they can be implemented with a single call
// instruction with the same parameter and return types.

RWByteAddressBuffer buf;

// CHECK-DAG: %dx.types.ResRet.[[STY:v[0-9]*i16]] = type { <[[NUM:[0-9]*]] x i16>
// CHECK-DAG: %dx.types.ResRet.[[ITY:v[0-9]*i32]] = type { <[[NUM]] x i32>
// CHECK-DAG: %dx.types.ResRet.[[LTY:v[0-9]*i64]] = type { <[[NUM]] x i64>

[numthreads(8,1,1)]
void main() {
  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // Capture opcode number.
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[buf]], i32 999, i32 undef, i32 [[OP:[0-9]*]]
  buf.Store(999, OP);

  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 0
  // CHECK: [[svec:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  vector<int16_t, NUM> sVec = buf.Load<vector<int16_t, NUM> >(0);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 1024
  // CHECK: [[usvec:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  vector<uint16_t, NUM> usVec = buf.Load<vector<uint16_t, NUM> >(1024);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 2048
  // CHECK: [[ivec:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  vector<int, NUM> iVec = buf.Load<vector<int, NUM> >(2048);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 3072
  // CHECK: [[uivec:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  vector<uint, NUM> uiVec = buf.Load<vector<uint, NUM> >(3072);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 4096
  // CHECK: [[lvec:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  vector<int64_t, NUM> lVec = buf.Load<vector<int64_t, NUM> >(4096);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 5120
  // CHECK: [[ulvec:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  vector<uint64_t, NUM> ulVec = buf.Load<vector<uint64_t, NUM> >(5120);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x i16> @dx.op.unary.[[STY]](i32 [[OP]], <[[NUM]] x i16> [[svec]])
  vector<int16_t, NUM> sRes = FUNC(sVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x i16> @dx.op.unary.[[STY]](i32 [[OP]], <[[NUM]] x i16> [[usvec]])
  vector<uint16_t, NUM> usRes = FUNC(usVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x i32> @dx.op.unary.[[ITY]](i32 [[OP]], <[[NUM]] x i32> [[ivec]])
  vector<int, NUM> iRes = FUNC(iVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x i32> @dx.op.unary.[[ITY]](i32 [[OP]], <[[NUM]] x i32> [[uivec]])
  vector<uint, NUM> uiRes = FUNC(uiVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x i64> @dx.op.unary.[[LTY]](i32 [[OP]], <[[NUM]] x i64> [[lvec]])
  vector<int64_t, NUM> lRes = FUNC(lVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x i64> @dx.op.unary.[[LTY]](i32 [[OP]], <[[NUM]] x i64> [[ulvec]])
  vector<uint64_t, NUM> ulRes = FUNC(ulVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  buf.Store<vector<int16_t, NUM> >(0, sRes);
  buf.Store<vector<uint16_t, NUM> >(1024, usRes);
  buf.Store<vector<int, NUM> >(2048, iRes);
  buf.Store<vector<uint, NUM> >(3072, uiRes);
  buf.Store<vector<int64_t, NUM> >(4096, lRes);
  buf.Store<vector<uint64_t, NUM> >(5120, ulRes);
}
