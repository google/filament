// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -E main %s -ast-dump-implicit | FileCheck %s

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixStoreToMemory 'void (__builtin_LinAlgMatrix {{.*}}, float const __attribute__((address_space(3))) (&)[64], unsigned int, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} memory 'float const __attribute__((address_space(3))) (&)[64]'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} stride 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} layout 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 410
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""

groupshared float SharedArr[64];

[shader("compute")]
[numthreads(1,1,1)]
void main() {
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat;
  __builtin_LinAlg_MatrixStoreToMemory(mat, SharedArr, 0, 0, 0);
}
