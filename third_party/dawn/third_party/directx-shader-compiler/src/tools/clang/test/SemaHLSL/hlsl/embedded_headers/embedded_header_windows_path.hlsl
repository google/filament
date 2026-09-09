// RUN: %dxc -T ps_6_0 -E main -M %s | FileCheck %s
// RUN: %dxc -T ps_6_0 -E main -verify %s

// Verify that an embedded HLSL header is still found when the user spells
// the include path using Windows-style separators (backslashes) rather
// than forward slashes.  The compiled-in map is keyed on POSIX-style
// relative paths, so the preprocessor must normalise the spelling before
// looking up the embedded header.

// expected-no-diagnostics

#include <dx\linalg.h>

float4 main() : SV_Target { return 0; }

// The dependency dump must list the canonical (forward-slash) virtual
// path even though the include used backslashes.
// CHECK: <built-in:hlsl>/dx/linalg.h
