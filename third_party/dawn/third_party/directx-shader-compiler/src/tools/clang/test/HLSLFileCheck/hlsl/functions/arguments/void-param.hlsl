// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK-NOT: error: empty parameter list defined with a typedef of 'void' not allowed in HLSL
// CHECK: void-param.hlsl:12:16: error: argument may not have 'void' type
// CHECK: void-param.hlsl:14:16: error: pointers are unsupported in HLSL
// CHECK: void-param.hlsl:16:10: error: 'void' as parameter must not have type qualifiers
// CHECK: void-param.hlsl:18:10: error: 'void' must be the first and only parameter if specified
// CHECK: void-param.hlsl:20:17: error: variadic arguments is unsupported in HLSL
// CHECK: void-param.hlsl:20:10: error: 'void' must be the first and only parameter if specified
// CHECK: void-param.hlsl:22:10: error: 'void' must be the first and only parameter if specified

void foo2(void a) {}

void foo2(void *p) {}

void foo3(const void) {}

void foo4(float a, void) {}

void foo5(void, ...) {}

void foo6(void, float a) {}

void foo1(void) {}

float4 main() : SV_TARGET {
 return 0;
}