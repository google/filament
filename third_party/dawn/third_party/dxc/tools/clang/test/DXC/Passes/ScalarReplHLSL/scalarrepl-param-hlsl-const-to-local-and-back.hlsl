// RUN: not %dxc -T ps_6_2 %s 2>&1 | FileCheck %s

// Validate that copying from static array to local, then back to static
// array does not crash the compiler. This was resulting in an invalid
// ReplaceConstantWithInst from ScalarReplAggregatesHLSL, which would
// result in referenced deleted instruction in a later pass.

// CHECK: error: Loop must have break.

static int arr1[10] = (int[10])0;
static int arr2[10] = (int[10])0;
static float result = 0;
ByteAddressBuffer buff : register(t0);

void foo() {
  int i = 0;
  if (buff.Load(0u)) {
    return;
  }
  arr2[i] = arr1[i];
  result = float(arr1[0]);
}

struct tint_symbol {
  float4 value : SV_Target0;
};

float main_inner() {
  foo();
  bool cond = false;
  while (true) {
    if (cond) { break; }
  }
  int arr1_copy[10] = arr1; // constant to local
  arr1 = arr1_copy; // local to constant
  foo();
  return result;
}

tint_symbol main() {
  float inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value.x = inner_result;
  return wrapper_result;
}
