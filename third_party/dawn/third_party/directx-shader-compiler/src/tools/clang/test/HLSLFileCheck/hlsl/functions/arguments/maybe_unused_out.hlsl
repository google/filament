// RUN: %dxc -T lib_6_4 %s -ast-dump | FileCheck %s
// Verify attribute annotation to opt out of uninitialized parameter analysis.

void UnusedOutput([maybe_unused] out int Val) {}


// CHECK: FunctionDecl {{.*}} UnusedOutput 'void (int &__restrict)'
// CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <col:34, col:42> col:42 Val 'int &__restrict'
// CHECK-NEXT: HLSLOutAttr {{0x[0-9a-fA-F]+}} <col:34>
// CHECK-NEXT: HLSLMaybeUnusedAttr {{0x[0-9a-fA-F]+}} <col:20>
