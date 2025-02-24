// RUN: %dxc -T lib_6_3 %s -verify
// RUN: not %dxc -T lib_6_3 %s -DEXTRA_ERROR -verify 2>&1 | FileCheck %s

foo(); // expected-error {{HLSL requires a type specifier for all declarations}}

#ifdef EXTRA_ERROR
baz(); // Woah!
#endif

// CHECK: error: 'error' diagnostics seen but not expected: 
// CHECK: File {{.*}}diagnostic-verifier.hlsl Line 7: HLSL requires a type specifier for all declarations
