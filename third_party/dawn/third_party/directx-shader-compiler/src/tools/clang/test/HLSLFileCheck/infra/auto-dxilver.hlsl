// Make sure each run line either works, or implicitly requires dxilver 1.8.
// If an implicit dxilver check is missing, we should see one of these errors.

// RUN: %dxc -T vs_6_8 %s | FileCheck %s
// This should implicitly require dxilver 1.8.

// RUN: %dxc -T vs_6_8 -Vd %s | FileCheck %s
// Even though this is using -Vd, the validator version being checked is the internal
// validator's version. If a pre-DXIL-1.8 DXC was used to run this test, we expect failure,
// since the internal validator will be the same version as the older DXC.
// The implicit dxilver logic should not skip the check when -Vd is used.
// CHECK-NOT: error: validator version {{.*}} does not support target profile.

// RUN: %dxc -T vs_6_0 -validator-version 1.8 %s | FileCheck %s
// Even though target is 6.0, the explicit -validator-version should add an
// implicit dxilver 1.8 requirement. The requirement should pass for DXCs that
// are newer than DXIL Version 1.8, since then, the internal validator's version will
// be sufficiently new for this check.
// CHECK-NOT: error: The module cannot be validated by the version of the validator currently attached.

// This error would occur if run against wrong compiler.
// CHECK-NOT: error: invalid profile

// Catch any other unexpected error cases.
// CHECK-NOT: error

// CHECK: define void @main()

void main() {}
