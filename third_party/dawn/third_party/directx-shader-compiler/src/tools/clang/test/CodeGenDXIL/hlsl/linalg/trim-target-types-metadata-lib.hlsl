// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_x -DLIB1 %s | FileCheck %s --check-prefix=LIB1
// RUN: %dxc -T lib_6_x -DLIB1 -Fo %t.lib1.dxbc %s
// RUN: %dxc -T lib_6_x -DLIB2 %s | FileCheck %s --check-prefix=LIB2
// RUN: %dxc -T lib_6_x -DLIB2 -Fo %t.lib2.dxbc %s
// RUN: %dxl -T cs_6_10 -E CSMain1 "%t.lib1.dxbc;%t.lib2.dxbc" | FileCheck %s --check-prefix=CSMAIN1
// RUN: %dxl -T cs_6_10 -E CSMain2 "%t.lib1.dxbc;%t.lib2.dxbc" | FileCheck %s --check-prefix=CSMAIN2
// RUN: %dxl -T cs_6_10 -E CSMain3 "%t.lib1.dxbc;%t.lib2.dxbc" | FileCheck %s --check-prefix=CSMAIN3

uint useMatrix1();
uint useMatrix2();
void useMatrix3();

// This test is using 2 LinAlgMatrix operations:
// - __builtin_LinAlg_FillMatrix - has the matrix as a return value
// - __builtin_LinAlg_MatrixLength - has the matrix as an argument
// This is done to verify that target types are correctly collected from both
// return values and arguments of LinAlgMatrix operations.

// ---- lib1 source code --- //
#ifdef LIB1

uint useMatrix1() {
  // Matrix<ComponentType::I32, 4, 5, MatrixUse::A, MatrixScope::ThreadGroup> m;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 4, 5, 0, 2)]] mat1;
  // mat1 = Matrix::Splat(5);
  __builtin_LinAlg_FillMatrix(mat1, 5);
  // return mat1.Length();
  return __builtin_LinAlg_MatrixLength(mat1);
}

uint useMatrix2() {
  // Matrix<ComponentType::F64, 2, 2, MatrixUse::B, MatrixScope::Wave> m;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(10, 4, 4, 1, 1)]] mat2;
  // Matrix::Splat(1)
  __builtin_LinAlg_FillMatrix(mat2, 1);
  // return mat2.Length();
  return __builtin_LinAlg_MatrixLength(mat2);
}

#endif

// ---- lib2 source code --- //
#ifdef LIB2

void useMatrix3() {
  //Matrix<ComponentType::U32, 6, 6, MatrixUse::Accumulator, MatrixScope::ThreadGroup> m;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(5, 6, 6, 2, 2)]] mat3;
  // mat3 = Matrix::Splat(5);
  __builtin_LinAlg_FillMatrix(mat3, 5);
}

RWBuffer<uint> Out;

[shader("compute")]
[numthreads(4,1,1)]
void CSMain1() {
  // no matrix used
}

[shader("compute")]
[numthreads(4,1,1)]
void CSMain2() {
  Out[0] = useMatrix1();
}

[shader("compute")]
[numthreads(4,1,1)]
void CSMain3() {
  Out[0] = useMatrix2();
  useMatrix3();
}

#endif

// Target types in lib1
// LIB1: !dx.targetTypes = !{![[TT1:.*]], ![[TT2:.*]]}
// LIB1: ![[TT1]] = !{%dx.types.LinAlgMatrixC4M4N5U0S2 undef, i32 4, i32 4, i32 5, i32 0, i32 2}
// LIB1: ![[TT2]] = !{%dx.types.LinAlgMatrixC10M4N4U1S1 undef, i32 10, i32 4, i32 4, i32 1, i32 1}

// Target types in lib2
// LIB2: !dx.targetTypes = !{![[TT3:.*]]}
// LIB2: ![[TT3]] = !{%dx.types.LinAlgMatrixC5M6N6U2S2 undef, i32 5, i32 6, i32 6, i32 2, i32 2}

// Target types in final module (should be only those that are used)

// CSMain1 doesn't use any matrix, so the target types should be filtered out from the final module.
// CSMAIN1-NOT: !dx.targetTypes

// CSMain2 uses one type of matrix
// CSMAIN2: !dx.targetTypes = !{!{{[0-9]+}}}
// CSMAIN2: !{{[0-9]+}} = !{%dx.types.LinAlgMatrixC4M4N5U0S2 undef, i32 4, i32 4, i32 5, i32 0, i32 2}

// CSMain3 uses two types of matrices
// CSMAIN3: !dx.targetTypes = !{!{{[0-9]+}}, !{{[0-9]+}}}
// CSMAIN3-DAG: !{{[0-9]+}} = !{%dx.types.LinAlgMatrixC10M4N4U1S1 undef, i32 10, i32 4, i32 4, i32 1, i32 1}
// CSMAIN3-DAG: !{{[0-9]+}} = !{%dx.types.LinAlgMatrixC5M6N6U2S2 undef, i32 5, i32 6, i32 6, i32 2, i32 2}
// CSMAIN3-NOT: !{%dx.types.LinAlgMatrixC10M4N4U1S1
