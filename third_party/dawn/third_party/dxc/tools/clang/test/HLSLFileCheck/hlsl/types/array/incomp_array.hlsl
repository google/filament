// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Verify no hang on incomplete array

// CHECK: %"$Globals" = type { i32, %struct.Special }
// CHECK: %struct.Special = type { <4 x float>, [3 x i32] }

typedef const int inta[];

// CHECK: @s_testa = internal unnamed_addr constant [3 x i32] [i32 1, i32 2, i32 3], align 4
static inta s_testa = {1, 2, 3};

int i;

struct Special {
  float4 member;
  int a[3];
};

Special c_special;

static const Special s_special = { { 1, 2, 3, 4}, { 5, 6, 7 } };

// CHECK: define <4 x float>
// CHECK: fn1
// @"\01?fn1{{[@$?.A-Za-z0-9_]+}}"
float4 fn1(in Special in1: SEMANTIC_IN) : SEMANTIC_OUT {
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(
  // CHECK: i32 0)
  // CHECK: extractvalue
  // CHECK: , 0
  // CHECK: getelementptr
  // CHECK: load i32, i32*
  // CHECK: sitofp i32
  // CHECK: fadd fast float
  return in1.member + (float)s_testa[i];
}

// CHECK: define <4 x float>
// CHECK: fn2
// @"\01?fn2{{[@$?.A-Za-z0-9_]+}}"
float4 fn2(in Special in1: SEMANTIC_IN) : SEMANTIC_OUT {
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(
  // CHECK: i32 0)
  // CHECK: extractvalue
  // CHECK: , 0
  // CHECK: getelementptr
  // CHECK: load i32, i32*
  // CHECK: sitofp i32
  // CHECK: fadd fast float
  return in1.member + (float)s_special.a[i];
}

// CHECK: define <4 x float>
// CHECK: fn3
// @"\01?fn3{{[@$?.A-Za-z0-9_]+}}"
float4 fn3(in Special in1: SEMANTIC_IN) : SEMANTIC_OUT {
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(
  // CHECK: i32 0)
  // CHECK: extractvalue
  // CHECK: , 0
  // CHECK: getelementptr
  // CHECK: load i32, i32*
  // CHECK: sitofp i32
  // CHECK: fadd fast float
  return in1.member + (float)in1.a[i];
}

// CHECK: define <4 x float>
// CHECK: fn4
// @"\01?fn4{{[@$?.A-Za-z0-9_]+}}"
float4 fn4(in Special in1: SEMANTIC_IN) : SEMANTIC_OUT {
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(
  // CHECK: i32 0)
  // CHECK: extractvalue
  // CHECK: , 0
  // CHECK: add
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(
  // CHECK: extractvalue
  // CHECK: , 0
  // CHECK: sitofp i32
  // CHECK: fadd fast float
  return in1.member + c_special.a[i];
}
