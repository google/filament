// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=isnan    -DOP=8  -DNUM=39 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=isinf    -DOP=9  -DNUM=38 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=isfinite -DOP=10 -DNUM=37 %s | FileCheck %s

// Test vector-enabled isspecial unary intrinsics that take float-like parameters and
// and are "trivial" in that they can be implemented with a single call.
// These return boolean vectors of the same size as their paraemter.

RWByteAddressBuffer buf;

// CHECK-DAG: %dx.types.ResRet.[[HTY:v[0-9]*f16]] = type { <[[NUM:[0-9]*]] x half>
// CHECK-DAG: %dx.types.ResRet.[[FTY:v[0-9]*f32]] = type { <[[NUM]] x float>

[numthreads(8,1,1)]
void main() {

  // Capture opcode number.
  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[buf]], i32 999, i32 undef, i32 [[OP:[0-9]*]]
  buf.Store(999, OP);

  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[HTY]] @dx.op.rawBufferVectorLoad.[[HTY]](i32 303, %dx.types.Handle [[buf]], i32 0
  // CHECK: [[hvec:%.*]] = extractvalue %dx.types.ResRet.[[HTY]] [[ld]], 0
  vector<float16_t, NUM> hVec = buf.Load<vector<float16_t, NUM> >(0);

  // Convergent markers prevent GVN removal of redundant annotateHandle calls.
  // CONV: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[FTY]] @dx.op.rawBufferVectorLoad.[[FTY]](i32 303, %dx.types.Handle [[buf]], i32 1024
  // CHECK: [[fvec:%.*]] = extractvalue %dx.types.ResRet.[[FTY]] [[ld]], 0
  vector<float, NUM> fVec = buf.Load<vector<float, NUM> >(1024);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // NOTE: This behavior will change with #7588
  // CHECK: [[tmp:%.*]] = fpext <[[NUM]] x half> [[hvec]] to <[[NUM]] x float>
  // CHECK: call <[[NUM]] x i1> @dx.op.isSpecialFloat.[[FTY]](i32 [[OP]], <[[NUM]] x float> [[tmp]])
  vector<bool, NUM> hRes = FUNC(hVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x i1> @dx.op.isSpecialFloat.[[FTY]](i32 [[OP]], <[[NUM]] x float> [[fvec]])
  vector<bool, NUM> fRes = FUNC(fVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  buf.Store<vector<bool, NUM> >(0, hRes);
  buf.Store<vector<bool, NUM> >(1024, fRes);
}
