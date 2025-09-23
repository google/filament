// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=max   -DOP=35 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=max   -DOP=35 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=min   -DOP=36 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=min   -DOP=36 -DNUM=1022 %s | FileCheck %s

// Test vector-enabled binary intrinsics that take float-like parameters and
// and are "trivial" in that they can be implemented with a single call
// instruction with the same parameter and return types.

RWByteAddressBuffer buf;

// CHECK-DAG: %dx.types.ResRet.[[HTY:v[0-9]*f16]] = type { <[[NUM:[0-9]*]] x half>
// CHECK-DAG: %dx.types.ResRet.[[FTY:v[0-9]*f32]] = type { <[[NUM]] x float>
// CHECK-DAG: %dx.types.ResRet.[[DTY:v[0-9]*f64]] = type { <[[NUM]] x double>

[numthreads(8,1,1)]
void main() {

  // Capture opcode number.
  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[buf]], i32 999, i32 undef, i32 [[OP:[0-9]*]]
  buf.Store(999, OP);

  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[HTY]] @dx.op.rawBufferVectorLoad.[[HTY]](i32 303, %dx.types.Handle [[buf]], i32 0
  // CHECK: [[hvec1:%.*]] = extractvalue %dx.types.ResRet.[[HTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[HTY]] @dx.op.rawBufferVectorLoad.[[HTY]](i32 303, %dx.types.Handle [[buf]], i32 512
  // CHECK: [[hvec2:%.*]] = extractvalue %dx.types.ResRet.[[HTY]] [[ld]], 0
  vector<float16_t, NUM> hVec1 = buf.Load<vector<float16_t, NUM> >(0);
  vector<float16_t, NUM> hVec2 = buf.Load<vector<float16_t, NUM> >(512);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[FTY]] @dx.op.rawBufferVectorLoad.[[FTY]](i32 303, %dx.types.Handle [[buf]], i32 2048
  // CHECK: [[fvec1:%.*]] = extractvalue %dx.types.ResRet.[[FTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[FTY]] @dx.op.rawBufferVectorLoad.[[FTY]](i32 303, %dx.types.Handle [[buf]], i32 2560
  // CHECK: [[fvec2:%.*]] = extractvalue %dx.types.ResRet.[[FTY]] [[ld]], 0
  vector<float, NUM> fVec1 = buf.Load<vector<float, NUM> >(2048);
  vector<float, NUM> fVec2 = buf.Load<vector<float, NUM> >(2560);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[DTY]] @dx.op.rawBufferVectorLoad.[[DTY]](i32 303, %dx.types.Handle [[buf]], i32 4096
  // CHECK: [[dvec1:%.*]] = extractvalue %dx.types.ResRet.[[DTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[DTY]] @dx.op.rawBufferVectorLoad.[[DTY]](i32 303, %dx.types.Handle [[buf]], i32 4608
  // CHECK: [[dvec2:%.*]] = extractvalue %dx.types.ResRet.[[DTY]] [[ld]], 0
  vector<double, NUM> dVec1 = buf.Load<vector<double, NUM> >(4096);
  vector<double, NUM> dVec2 = buf.Load<vector<double, NUM> >(4608);

  // Test simple matching type overloads.

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x half> @dx.op.binary.[[HTY]](i32 [[OP]], <[[NUM]] x half> [[hvec1]], <[[NUM]] x half> [[hvec2]])
  vector<float16_t, NUM> hRes = FUNC(hVec1, hVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x float> @dx.op.binary.[[FTY]](i32 [[OP]], <[[NUM]] x float> [[fvec1]], <[[NUM]] x float> [[fvec2]])
  vector<float, NUM> fRes = FUNC(fVec1, fVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x double> @dx.op.binary.[[DTY]](i32 [[OP]], <[[NUM]] x double> [[dvec1]], <[[NUM]] x double> [[dvec2]])
  vector<double, NUM> dRes = FUNC(dVec1, dVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  buf.Store<vector<float16_t, NUM> >(0, hRes);
  buf.Store<vector<float, NUM> >(2048, fRes);
  buf.Store<vector<double, NUM> >(4096, dRes);
}
