// RUN: %dxc -T cs_6_9 -E main %s -verify

RWByteAddressBuffer Buf;
groupshared float SharedArr[64];

[numthreads(4,1,1)]
void main() {
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 5, 4, 0, 0)]] mat;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 1, 1, 1, 1)]] mat2;
  float4 vec1 = {1,2,3,4};
  float4 vec2 = {4,3,2,1};
  float4 result;
  uint elem;

  // expected-error@+1{{intrinsic __builtin_LinAlg_CopyConvertMatrix potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_CopyConvertMatrix(mat, mat2, true);

  // expected-error@+1{{intrinsic __builtin_LinAlg_FillMatrix potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_FillMatrix(mat, 1);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixAccumulate potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixAccumulate(mat2, mat, mat);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixAccumulateToDescriptor potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixAccumulateToDescriptor(mat, Buf, 9, 8, 7, 4);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixGetCoordinate potentially used by ''main'' requires shader model 6.10 or greater}}
  uint2 coord = __builtin_LinAlg_MatrixGetCoordinate(mat, 0);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixGetElement potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixGetElement(elem, mat, 0);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixLength potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixLength(mat);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixLoadFromDescriptor potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixLoadFromDescriptor(mat, Buf, 1, 1, 1, 4);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixMatrixMultiply potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixMatrixMultiply(mat2, mat, mat);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixMatrixMultiplyAccumulate potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixMatrixMultiplyAccumulate(mat, mat2, mat2, mat);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixOuterProduct potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixOuterProduct(mat, vec1, vec2);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixQueryAccumulatorLayout potentially used by ''main'' requires shader model 6.10 or greater}}
  uint layout = __builtin_LinAlg_MatrixQueryAccumulatorLayout();

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixSetElement potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixSetElement(mat, mat, 1, 1);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixStoreToDescriptor potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixStoreToDescriptor(mat, Buf, 1, 1, 1, 4);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixVectorMultiply potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixVectorMultiply(result, mat, true, vec1, 1);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixVectorMultiplyAdd potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixVectorMultiplyAdd(result, mat, true, vec1, 1, vec2);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixAccumulateToMemory potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixAccumulateToMemory(mat, SharedArr, 0, 0, 0, 0);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixLoadFromMemory potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixLoadFromMemory(mat, SharedArr, 0, 0, 0);

  // expected-error@+1{{intrinsic __builtin_LinAlg_MatrixStoreToMemory potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_MatrixStoreToMemory(mat, SharedArr, 0, 0, 0);

  // expected-error@+1{{intrinsic __builtin_LinAlg_Convert potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_Convert(result, vec1, 1, 1);

  // expected-error@+1{{intrinsic __builtin_LinAlg_VectorAccumulateToDescriptor potentially used by ''main'' requires shader model 6.10 or greater}}
  __builtin_LinAlg_VectorAccumulateToDescriptor(Buf, 1, 64, vec1);
}
