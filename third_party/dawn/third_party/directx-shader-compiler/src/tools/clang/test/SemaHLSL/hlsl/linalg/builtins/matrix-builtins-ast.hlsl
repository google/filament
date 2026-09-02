// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -E main %s -ast-dump-implicit | FileCheck %s

RWByteAddressBuffer Buf;
groupshared float SharedArr[64];

[shader("compute")]
[numthreads(1,1,1)]
void main() {
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 5, 4, 2, 2)]] mat1;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat2;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 5, 4, 2, 2)]] mat3;

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_CopyConvertMatrix 'void (__builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}}, bool)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} source '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} transpose 'bool'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 401
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_CopyConvertMatrix(mat2, mat1, true);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_FillMatrix 'void (__builtin_LinAlgMatrix & {{.*}}, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret '__builtin_LinAlgMatrix &&__restrict {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} value 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 402
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_FillMatrix(mat1, 15);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixAccumulate 'void (__builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}})' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrixC '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrixLHS '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrixRHS '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 411
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixAccumulate(mat1, mat2, mat3);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixAccumulateToDescriptor 'void (__builtin_LinAlgMatrix {{.*}}, RWByteAddressBuffer, unsigned int, unsigned int, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} buf 'RWByteAddressBuffer'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} stride 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} layout 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} align 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 415
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixAccumulateToDescriptor(mat1, Buf, 1, 2, 3, 4);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixAccumulateToMemory 'void (__builtin_LinAlgMatrix {{.*}}, float const __attribute__((address_space(3))) (&)[64], unsigned int, unsigned int, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} memory 'float const __attribute__((address_space(3))) (&)[64]'
// CHECK-NEXT: ParmVarDecl {{.*}} targetType 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} stride 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} layout 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 416
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixAccumulateToMemory(mat1, SharedArr, 0, 0, 0, 0);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixGetCoordinate 'vector<uint, 2> (__builtin_LinAlgMatrix {{.*}}, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} threadLocalIndex 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 403
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  uint2 coord = __builtin_LinAlg_MatrixGetCoordinate(mat1, 0);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixGetElement 'void (unsigned int &, __builtin_LinAlgMatrix {{.*}}, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret 'unsigned int &&__restrict'
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} threadLocalIndex 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 404
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  uint elem1;
  __builtin_LinAlg_MatrixGetElement(elem1, mat1, 3);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixGetElement 'void (float &, __builtin_LinAlgMatrix {{.*}}, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret 'float &&__restrict'
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} threadLocalIndex 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 404
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  float elem2;
  __builtin_LinAlg_MatrixGetElement(elem2, mat1, 4);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixLength 'unsigned int (__builtin_LinAlgMatrix {{.*}})' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 405
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixLength(mat1);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixLoadFromDescriptor 'void (__builtin_LinAlgMatrix & {{.*}}, RWByteAddressBuffer, unsigned int, unsigned int, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret '__builtin_LinAlgMatrix &&__restrict {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} buf 'RWByteAddressBuffer'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} stride 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} layout 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} align 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 406
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixLoadFromDescriptor(mat1, Buf, 0, 0, 0, 4);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixLoadFromMemory 'void (__builtin_LinAlgMatrix {{.*}}, float const __attribute__((address_space(3))) (&)[64], unsigned int, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} memory 'float const __attribute__((address_space(3))) (&)[64]'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} stride 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} layout 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 407
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixLoadFromMemory(mat1, SharedArr, 0, 0, 0);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixMatrixMultiply 'void (__builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}})' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrixC '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrixA '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrixB '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 412
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixMatrixMultiply(mat1, mat2, mat3);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixMatrixMultiplyAccumulate 'void (__builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}})' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrixR '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrixA '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrixB '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrixC '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 413
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixMatrixMultiplyAccumulate(mat1, mat2, mat3, mat1);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixOuterProduct 'void (__builtin_LinAlgMatrix {{.*}}, vector<int, 4>, vector<int, 4>)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} vecA 'vector<int, 4>':'vector<int, 4>'
// CHECK-NEXT: ParmVarDecl {{.*}} vecB 'vector<int, 4>':'vector<int, 4>'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 417
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  int4 vecA = {1,2,3,4};
  int4 vecB = {1,2,3,4};
  __builtin_LinAlg_MatrixOuterProduct(mat1, vecA, vecB);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixQueryAccumulatorLayout 'unsigned int ()' extern
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 414
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  uint layout = __builtin_LinAlg_MatrixQueryAccumulatorLayout();

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixSetElement 'void (__builtin_LinAlgMatrix {{.*}}, __builtin_LinAlgMatrix {{.*}}, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} threadLocalIndex 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} value 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 408
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixSetElement(mat2, mat1, 1, 1);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixStoreToDescriptor 'void (__builtin_LinAlgMatrix {{.*}}, RWByteAddressBuffer, unsigned int, unsigned int, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} buf 'RWByteAddressBuffer'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} stride 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} layout 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} align 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 409
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_MatrixStoreToDescriptor(mat1, Buf, 1, 2, 3, 4);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixStoreToMemory 'void (__builtin_LinAlgMatrix {{.*}}, float const __attribute__((address_space(3))) (&)[64], unsigned int, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} matrix '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} memory 'float const __attribute__((address_space(3))) (&)[64]'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} stride 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} layout 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 410
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""

  __builtin_LinAlg_MatrixStoreToMemory(mat1, SharedArr, 0, 0, 0);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixVectorMultiply 'void (vector<float, 4> &, __builtin_LinAlgMatrix {{.*}}, vector<float, 4>, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret 'vector<float, 4> &&__restrict'
// CHECK-NEXT: ParmVarDecl {{.*}} mat '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} isOutputSigned 'bool'
// CHECK-NEXT: ParmVarDecl {{.*}} input 'vector<float, 4>':'vector<float, 4>'
// CHECK-NEXT: ParmVarDecl {{.*}} inputInterp 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 418
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  float4 vec = {1,2,3,4};
  float4 result;
  __builtin_LinAlg_MatrixVectorMultiply(result, mat1, true, vec, 1);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_MatrixVectorMultiplyAdd 'void (vector<float, 4> &, __builtin_LinAlgMatrix {{.*}}, vector<float, 4>, unsigned int, vector<float, 4>)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret 'vector<float, 4> &&__restrict'
// CHECK-NEXT: ParmVarDecl {{.*}} mat '__builtin_LinAlgMatrix {{.*}}'
// CHECK-NEXT: ParmVarDecl {{.*}} isOutputSigned 'bool'
// CHECK-NEXT: ParmVarDecl {{.*}} input 'vector<float, 4>':'vector<float, 4>'
// CHECK-NEXT: ParmVarDecl {{.*}} inputInterp 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} bias 'vector<float, 4>':'vector<float, 4>'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 419
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  float4 input = {1,2,3,4};
  float4 bias = {5,6,7,8};
  __builtin_LinAlg_MatrixVectorMultiplyAdd(result, mat1, true, input, 1, bias);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_Convert 'void (vector<int, 4> &, vector<float, 4>, unsigned int, unsigned int)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} ret 'vector<int, 4> &&__restrict'
// CHECK-NEXT: ParmVarDecl {{.*}} vec 'vector<float, 4>':'vector<float, 4>'
// CHECK-NEXT: ParmVarDecl {{.*}} input_interp 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} output_interp 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 422
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  int4 result2;
  __builtin_LinAlg_Convert(result2, vec, 0, 1);

// CHECK: FunctionDecl {{.*}} implicit used __builtin_LinAlg_VectorAccumulateToDescriptor 'void (RWByteAddressBuffer, unsigned int, unsigned int, vector<float, 4>)' extern
// CHECK-NEXT: ParmVarDecl {{.*}} buf 'RWByteAddressBuffer'
// CHECK-NEXT: ParmVarDecl {{.*}} offset 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} align 'unsigned int'
// CHECK-NEXT: ParmVarDecl {{.*}} vec 'vector<float, 4>':'vector<float, 4>'
// CHECK-NEXT: HLSLIntrinsicAttr {{.*}} Implicit "op" "" 423
// CHECK-NEXT: AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
  __builtin_LinAlg_VectorAccumulateToDescriptor(Buf, 10, 64, input);
}
