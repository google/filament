// RUN: %dxc -I %hlsl_headers -T lib_6_9 %s | FileCheck %s

#include <dx/linalg.h>

ByteAddressBuffer Buf;

export float4 Test1(float4 input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_FLOAT16, 4, 4, MATRIX_LAYOUT_MUL_OPTIMAL> matrix = {Buf,
                                                                          0, 0};
  VectorRef<DATA_TYPE_FLOAT16> biasVector = {Buf, 256};

  InterpretedVector<float, 4, DATA_TYPE_FLOAT16> theVector = {input};

  // CHECK: %{{.+}} = call <4 x float> @dx.op.matVecMulAdd.v4f32.v4f32(i32 306, <4 x float> %{{.+}}, i1 false, i32 8, %dx.types.Handle [[RES:%.+]], i32 0, i32 8, i32 4, i32 4, i32 2, i1 false, i32 0, %dx.types.Handle [[RES]], i32 256, i32 8, i1 false)
  return MulAdd<float>(
      matrix, theVector,
      biasVector);
}

export float4 Test2(float4 input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_FLOAT16, 4, 4, MATRIX_LAYOUT_MUL_OPTIMAL, true> matrix = {
      Buf, 0, 0};
  VectorRef<DATA_TYPE_FLOAT16> biasVector = {Buf, 256};

  InterpretedVector<float, 4, DATA_TYPE_FLOAT16> theVector = {input};

  // CHECK: %{{.+}} = call <4 x float> @dx.op.matVecMulAdd.v4f32.v4f32(i32 306, <4 x float> %{{.+}}, i1 false, i32 8, %dx.types.Handle [[RES:%.+]], i32 0, i32 8, i32 4, i32 4, i32 2, i1 true, i32 0, %dx.types.Handle [[RES]], i32 256, i32 8, i1 false)
  return MulAdd<float>(
      matrix, theVector,
      biasVector);
}

export float4 Test3(float4 input) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_FLOAT16, 4, 4, MATRIX_LAYOUT_MUL_OPTIMAL, true> matrix = {
      Buf, 0, 0};
  VectorRef<DATA_TYPE_FLOAT16> biasVector = {Buf, 256};

  // CHECK: %{{.+}} = call <4 x float> @dx.op.matVecMulAdd.v4f32.v4f32(i32 306, <4 x float> %{{.+}}, i1 false, i32 8, %dx.types.Handle [[RES:%.+]], i32 0, i32 8, i32 4, i32 4, i32 2, i1 true, i32 0, %dx.types.Handle [[RES]], i32 256, i32 8, i1 false)
  return MulAdd<float>(
      matrix, MakeInterpretedVector<DATA_TYPE_FLOAT16>(input),
      biasVector);
}

namespace ProposalExample {

ByteAddressBuffer model;

vector<float, 3> ApplyNeuralMaterial(vector<half, 8> inputVector) {
  using namespace dx::linalg;

  MatrixRef<DATA_TYPE_FLOAT8_E4M3, 32, 8, MATRIX_LAYOUT_MUL_OPTIMAL> matrix0 = {
      model, 0, 0};

  VectorRef<DATA_TYPE_FLOAT16> biasVector0 = {model, 1024};

  MatrixRef<DATA_TYPE_FLOAT8_E4M3, 32, 32, MATRIX_LAYOUT_MUL_OPTIMAL> matrix1 =
      {model, 2048, 0};

  VectorRef<DATA_TYPE_FLOAT16> biasVector1 = {model, 3072};

  MatrixRef<DATA_TYPE_FLOAT8_E4M3, 3, 32, MATRIX_LAYOUT_MUL_OPTIMAL> matrix2 = {
      model, 4096, 0};

  VectorRef<DATA_TYPE_FLOAT16> biasVector2 = {model, 5120};

  vector<half, 32> layer0 = MulAdd<half>(
      matrix0, MakeInterpretedVector<DATA_TYPE_FLOAT8_E4M3>(inputVector),
      biasVector0);
  layer0 = max(layer0, 0);

  vector<half, 32> layer1 = MulAdd<half>(
      matrix1, MakeInterpretedVector<DATA_TYPE_FLOAT8_E4M3>(layer0),
      biasVector1);
  layer1 = max(layer1, 0);

  vector<float, 3> output = MulAdd<float>(
      matrix2, MakeInterpretedVector<DATA_TYPE_FLOAT8_E4M3>(layer1),
      biasVector2);
  output = exp(output);

  return output;
}

} // namespace ProposalExample
