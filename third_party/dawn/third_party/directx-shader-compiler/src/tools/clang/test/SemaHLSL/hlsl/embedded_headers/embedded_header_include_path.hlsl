// RUN: %dxc -T ps_6_0 -E main -I %S/Inputs -M %s | FileCheck %s
// RUN: %dxc -T ps_6_0 -E main -I %S/Inputs -verify %s

// When -I points at a directory that contains a header whose name matches
// one of the compiled-in HLSL headers, an angle-bracket #include must
// resolve to the on-disk file rather than the embedded copy.  This lets
// users inspect or override the bundled headers by dropping a same-named
// file into a directory on the include search path.

// expected-no-diagnostics

#include <enable_if>

#ifndef HLSL_OVERLAY_ENABLE_IF
#error "Expected the on-disk overlay enable_if to take precedence over the embedded copy."
#endif

float4 main() : SV_Target { return 0; }

// The dependency dump must list the on-disk overlay path and must not
// reference the virtual built-in:hlsl filename used for embedded headers.
// CHECK-NOT: <built-in:hlsl>
// CHECK: Inputs{{[/\\]}}enable_if
