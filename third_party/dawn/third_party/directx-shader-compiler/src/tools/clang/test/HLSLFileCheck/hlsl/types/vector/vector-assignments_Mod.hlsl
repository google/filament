// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T vs_2_0 vector-assignments.hlsl

float pick_one(float2 f2) {
  return f2.x;
}

void main() {

// No initialization.
float2 f2_none;

// Direct initialization fails.

// Initialization list with members.
float2 f2_all = { 0.1f, 0.2f };

// Initialization list with insufficient members fails.

// Initialization list with too many members fails.

// Initialization list with different element types.
float2 f2_ints = { 1, 2 };

// Initialization list with different element types.
double d = 0.123;
float2 f2_int_double = { 1, d };

// Initialization list with packed element.
float2 f2_f2 = { f2_all };

// Initialization list with mixed packed elements.
float3 f3_f2_f = { f2_all, 0.1f };

// Initialization list with too many mixed packed elements fails.

// Constructor with wrong element count fails.

// Construct with exact number.
float2 f2c_f_f = float2(0.1f, 0.2f);

// Construct with packed value.
float2 f2c_f2 = float2(f2c_f_f);

// Construct with mixed values.
float3 f3c_f2_f = float3(f2c_f_f, 1);

// *assignments* don't mind if they are narrowing, but warn.
float2 f2a_f2_f = f3c_f2_f; // expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}}
float2 f2c_f2_f = float3(f2c_f_f, 1); // expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}}

// *assignments* do mind if they are widening.

// initializers only work on assignment.

// constructors work without special cases.
pick_one(float2(0.1f, 0.2f));

// TODO: this is in fact allowed, but we need HLSL implicit conversions during argument matching;
// this can be enabled, but it's causing additional problems because there is no HLSL-style ranking
// of conversions (eg, float4->float4 is better than float4->float3).
// pick_one(uint2(1, 2));
}
