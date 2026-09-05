// Note: Base this on highest the experimental version, or released+1.
// REQUIRES: dxil-1-10
// RUN: %dxc %s -Tlib_6_10 -Vd -validator-version 0.0 -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,EXP
// RUN: %dxc %s -Tlib_6_10 -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,EXP
// RUN: %dxc %s -Tlib_6_10 -validator-version 1.10 -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,EXP

// No experimental RDAT for released shader models even with default or latest
// validator versions.
// RUN: %dxc %s -Tlib_6_9 -Fo %t
// RUN: %dxa %t -dumprdat | FileCheck %s -check-prefixes=CHECK,NOEXP
// RUN: %dxc %s -Tlib_6_9 -validator-version 1.10 -Fo %t
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
