// RUN: %dxc -E main -T ps_6_0  %s  | FileCheck %s
// RUN: %dxc -E main -T ps_6_6  %s  | FileCheck %s
// RUN: %dxc -E main -T lib_6_x  %s  | FileCheck %s

// Make sure cbuffer use has been translated
// CHECK-NOT: @dx.op.cbufferLoadLegacy.f32(i32 59,
// CHECK-NOT: extractvalue %dx.types.CBufRet.f32

// Make sure both a and b are used from correct buffer load
// CHECK: @dx.op.bufferLoad.i32(i32 68,
// CHECK:extractvalue %dx.types.ResRet.i32 %{{.*}}, 0
// CHECK:extractvalue %dx.types.ResRet.i32 %{{.*}}, 1

// Make sure cbuffer use has been translated
// CHECK-NOT: @dx.op.cbufferLoadLegacy.f32(i32 59,
// CHECK-NOT: extractvalue %dx.types.CBufRet.f32

struct S {
  float a;
  float b;
};

TextureBuffer<S> c[2]: register(t2, space5);;

export TextureBuffer<S> getTBV(uint i) {
  return c[i];
}

export S getS(TextureBuffer<S> tbv) {
  return tbv;
}

float main(uint i : IN) : SV_Target {
  return c[i].a + getS(getTBV(i)).b;
}
