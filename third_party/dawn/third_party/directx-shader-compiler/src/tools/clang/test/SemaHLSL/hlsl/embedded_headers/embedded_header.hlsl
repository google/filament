// RUN: %dxc -T ps_6_0 -E main -M %s | FileCheck %s
// RUN: %dxc -T ps_6_0 -E main -verify %s

// Verify that the bundled HLSL header tools/clang/lib/Headers/hlsl/enable_if
// is found via the angled-#include "embedded headers" path even when no -I
// search path points at the hlsl/ directory.

// expected-no-diagnostics

#include <enable_if>

float4 main() : SV_Target { return 0; }

// CHECK: <built-in:hlsl>/enable_if

