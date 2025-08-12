// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types %s -verify

#include <dx/linalg.h>

using namespace dx::linalg;

ByteAddressBuffer input_vector_buffer;
RWByteAddressBuffer accumulate_buffer;
ByteAddressBuffer constants_buffer;

// Check for input vectors aren't the same component type
void test_invalid_input_vector_component_type() {

  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL;
  const uint matrix_stride = 0;

  vector<float, 4> input_vector_0_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<float, 16> input_vector_1_0 = input_vector_buffer.Load<vector<float, 16> >(0);

      // expected-no-diagnostics@+1
  __builtin_OuterProductAccumulate(input_vector_0_0, input_vector_1_0,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);

  vector<int, 32> input_vector_0_1 = input_vector_buffer.Load<vector<int, 32> >(0);
  vector<int ,16> input_vector_1_1 = input_vector_buffer.Load<vector<int, 16> >(0);

     // expected-no-diagnostics@+1
  __builtin_OuterProductAccumulate(input_vector_0_1, input_vector_1_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);

  vector<uint, 4> input_vector_0_2 = input_vector_buffer.Load<vector<uint, 4> >(0);
  vector<uint, 16> input_vector_1_2 = input_vector_buffer.Load<vector<uint, 16> >(0);

  // expected-no-diagnostics@+1
  __builtin_OuterProductAccumulate(input_vector_0_2, input_vector_1_2,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);
}

// Check for non constant matrix stride
void test_non_constant_matrix_stride() {

  vector<float, 4> input_vector_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<float, 4> input_vector_1 = input_vector_buffer.Load<vector<float, 4> >(0);
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL;

  const uint matrix_stride = constants_buffer.Load<uint>(0);

  // expected-no-diagnostics@+4
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);
}

// Check for matrix stride is not a valid value

