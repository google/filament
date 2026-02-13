// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types %s -verify

#include <dx/linalg.h>

using namespace dx::linalg;

ByteAddressBuffer input_vector_buffer;
RWByteAddressBuffer accumulate_buffer;
ByteAddressBuffer constants_buffer;

// Check if input vectors aren't the same component type
void test_invalid_input_vector_component_type() {

  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL;
  const uint matrix_stride = 0;

  vector<float, 4> input_vector_0_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<uint, 4> input_vector_1_0 = input_vector_buffer.Load<vector<uint, 4> >(0);

  // expected-error@+1 {{input vectors of outerproductaccumulate must have the same element type}}
  __builtin_OuterProductAccumulate(input_vector_0_0, input_vector_1_0,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);

  vector<int, 4> input_vector_0_1 = input_vector_buffer.Load<vector<int, 4> >(0);
  vector<float, 4> input_vector_1_1 = input_vector_buffer.Load<vector<float, 4> >(0);

  // expected-error@+1 {{input vectors of outerproductaccumulate must have the same element type}}
  __builtin_OuterProductAccumulate(input_vector_0_1, input_vector_1_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);
}

// Check for non constant matrix interpretation
void test_non_constant_matrix_interpretation() {

  vector<float, 4> input_vector_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<float, 4> input_vector_1 = input_vector_buffer.Load<vector<float, 4> >(0);
  const uint matrix_offset = 0;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL;
  const uint matrix_stride = 0;

  const uint matrix_interpretation = constants_buffer.Load<uint>(0);

  // expected-error@+3 {{expression is not an integer constant expression}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);
}

// Check for matrix interpretation is not a valid value
void test_invalid_matrix_interpretation() {

  vector<float, 4> input_vector_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<float, 4> input_vector_1 = input_vector_buffer.Load<vector<float, 4> >(0);
  const uint matrix_offset = 0;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL;
  const uint matrix_stride = 0;

  const uint matrix_interpretation = 0;

  // expected-error@+3 {{0 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_2 = 1;

  // expected-error@+3 {{1 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_2, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_3 = 6;

  // expected-error@+3 {{6 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_3, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_4 = 7;

  // expected-error@+3 {{7 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_4, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_5 = 10;

  // expected-error@+3 {{10 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_5, matrix_layout,
                                  matrix_stride); 

  const uint matrix_interpretation_6 = 11;

  // expected-error@+3 {{11 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_6, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_7 = 12;

  // expected-error@+3 {{12 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_7, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_8 = 13;

  // expected-error@+3 {{13 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_8, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_9 = 14;

  // expected-error@+3 {{14 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_9, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_10 = 15;

  // expected-error@+3 {{15 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_10, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_11 = 16;

  // expected-error@+3 {{16 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_11, matrix_layout,
                                  matrix_stride); 

  const uint matrix_interpretation_12 = DataType::DATA_TYPE_SINT8_T4_PACKED;

  // expected-error@+3 {{17 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_12, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_13 = DataType::DATA_TYPE_UINT8_T4_PACKED;

  // expected-error@+3 {{18 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_13, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_14 = 23;

  // expected-error@+3 {{23 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_14, matrix_layout,
                                  matrix_stride);

  const uint matrix_interpretation_15 = 100;

  // expected-error@+3 {{100 is an invalid memory interpretation value}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation_15, matrix_layout,
                                  matrix_stride);                   
                              
}

// Check for matrix layout is not a constant parameter
void test_non_constant_matrix_layout() {

  vector<float, 4> input_vector_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<float, 4> input_vector_1 = input_vector_buffer.Load<vector<float, 4> >(0);
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_stride = 0;

  const uint matrix_layout = constants_buffer.Load<uint>(0);

  // expected-error@+3 {{expression is not an integer constant expression}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);
}

// Check for matrix layout is not a valid value
void test_invalid_matrix_layout() {

  vector<float, 4> input_vector_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<float, 4> input_vector_1 = input_vector_buffer.Load<vector<float, 4> >(0);
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32; 
  const uint matrix_stride = 0;

  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;

  // expected-error@+3 {{matrix layout for outerproductaccumulate must be 3}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);

  const uint matrix_layout_2 = MatrixLayout::MATRIX_LAYOUT_COLUMN_MAJOR;

  // expected-error@+3 {{matrix layout for outerproductaccumulate must be 3}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout_2,
                                  matrix_stride);

  const uint matrix_layout_3 = MatrixLayout::MATRIX_LAYOUT_MUL_OPTIMAL;

  // expected-error@+3 {{matrix layout for outerproductaccumulate must be 3}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout_3,
                                  matrix_stride);                               
                                  
}

// Check for matrix stride is zero, if constant
void test_zero_matrix_stride() {

  vector<float, 4> input_vector_0 = input_vector_buffer.Load<vector<float, 4> >(0);
  vector<float, 4> input_vector_1 = input_vector_buffer.Load<vector<float, 4> >(0);
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL;

  const uint matrix_stride = 16;

  // expected-error@+4 {{for optimal matrix layout, matrix stride must be 0}}
  __builtin_OuterProductAccumulate(input_vector_0, input_vector_1,
                                  accumulate_buffer, matrix_offset,
                                  matrix_interpretation, matrix_layout,
                                  matrix_stride);
}
