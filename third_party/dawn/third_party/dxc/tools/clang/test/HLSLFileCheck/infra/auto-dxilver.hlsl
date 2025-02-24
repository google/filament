// Make sure each run line either works, or implicitly requires dxilver 1.8.
// If an implicit dxilver check is missing, we should see one of these errors.

// RUN: %dxc -T vs_6_8 %s | FileCheck %s
// This should implicitly require dxilver 1.8.

// RUN: %dxc -T vs_6_8 -Vd %s | FileCheck %s
// Even though this is using -Vd, the validator version is set by the available
// validator.  If that isn't version 1.8 or above, we'll see an error.
// The implicit dxilver logic should not skip the check when -Vd is used.
// CHECK-NOT: error: validator version {{.*}} does not support target profile.

// RUN: %dxc -T vs_6_0 -validator-version 1.8 %s | FileCheck %s
// Even though target is 6.0, the explicit -validator-version should add an
// implicit dxilver 1.8 requirement.
// CHECK-NOT: error: The module cannot be validated by the version of the validator currently attached.

// This error would occur if run against wrong compiler.
// CHECK-NOT: error: invalid profile

// Catch any other unexpected error cases.
// CHECK-NOT: error

// RUN: %dxc -T vs_6_8 -select-validator internal %s | FileCheck %s
// This should always be run, and always succeed.
// CHECK: define void @main()

void main() {}
