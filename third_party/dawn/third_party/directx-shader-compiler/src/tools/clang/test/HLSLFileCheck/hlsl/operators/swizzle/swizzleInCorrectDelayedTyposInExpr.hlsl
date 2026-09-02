// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Test hlsl::LookupVectorMemberExprForHLSL codepath
// This appears to only be called from a specific CorrectDelayedTyposInExpr code path.
// It requires a number of conditions to get there.

// CHECK: error: use of undeclared identifier 'some_var_2'; did you mean 'some_var_1'
// CHECK: error: use of undeclared identifier 'some_var_3'

float3 some_fn(float4 a, float b) { return b; }
float4 foo(int i) { return i; }
float3 repro() {
  // some_var_1 needs to be close to the undefined some_var_2,
  // used in a dependent call expression,
  // next to a swizzle on another call expression,
  // so it can try to resolve the possible typos for suggested replacements.
  // This eventually reaches the .xyz on foo(0),
  // to resolve the vector member expression
  // using hlsl::LookupVectorMemberExprForHLSL
  float4 some_var_1;
  return some_fn(some_var_2, foo(0).xyz) + some_other_fn(1.0.xxxx, 0.0.xxxx, some_var_3);
}
float3 main(float4 input : IN) : OUT {
  return repro();
}
