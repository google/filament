// Note: Add RUN lines for newer released shader models to this test when available.
// REQUIRES: dxil-1-9
// RUN: %dxc %s -Tlib_6_x -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,EXP
// RUN: %dxc %s -Tlib_6_9 -Vd -validator-version 0.0 -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,EXP
// RUN: %dxc %s -Tlib_6_9 -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,NOEXP
// RUN: %dxc %s -Tlib_6_9 -validator-version 1.9 -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,NOEXP

// Make sure experimental RDAT is not emitted for released shader models,
// unless the validator version is 0.0 (no validation supported). Validator
// version 0.0 is implied for the offline linking target lib_6_x.

// CHECK: DxilRuntimeData
// EXP: PSInfoTable
// NOEXP-NOT: PSInfoTable

[shader("pixel")]
float4 main() : SV_Target {
    return float4(1.0, 0.0, 0.0, 1.0);
}
