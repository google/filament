// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types %s -verify

#include <dx/linalg.h>

using namespace dx::linalg;

ByteAddressBuffer input_vector_buffer;
ByteAddressBuffer matrix_buffer; 
RWByteAddressBuffer output_vector_buffer;
ByteAddressBuffer const_buffer;

// Output vector, isUnsigned mismatch
void test_valid_output_vector_type() {

    vector<float, 4> input_vector = input_vector_buffer.Load<vector<float, 4> >(0);
    const uint is_input_unsigned = 0;
    const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
    const uint matrix_offset = 0;
    const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
    const uint matrix_dimM = 4;
    const uint matrix_dimK = 4;
    const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
    const bool matrix_is_transposed = false;
    const uint matrix_stride = 64;

    vector<uint, 4> output_vector_0;
    const uint is_output_unsigned_0 = 1;

    // expected-no-diagnostics@+1
    __builtin_MatVecMul(output_vector_0, is_output_unsigned_0, input_vector,
        is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
        matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
        matrix_is_transposed, matrix_stride);

    vector<int32_t, 4> output_vector_1;
    const uint is_output_unsigned_1 = 0;

    // expected-no-diagnostics@+1
    __builtin_MatVecMul(output_vector_1, is_output_unsigned_1, input_vector,
        is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
        matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
        matrix_is_transposed, matrix_stride);

    vector<float, 4> output_vector_2;
    const uint is_output_unsigned_2 = 0;

    // expected-no-diagnostics@+1
    __builtin_MatVecMul(output_vector_2, is_output_unsigned_2, input_vector,
        is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
        matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
        matrix_is_transposed, matrix_stride);
}

void test_valid_is_output_unsigned_non_const() {

  vector<uint, 4> output_vector_0;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint is_output_unsigned_0 = 1;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector_0, is_output_unsigned_0, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Input vector is incorrect type
void test_valid_input_vector_type() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

    vector<int32_t, 4> input_vector_0 =
      input_vector_buffer.Load<vector<int32_t, 4> >(0);
    const uint is_input_unsigned_0 = 0;

 // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned_0, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

    vector<uint32_t, 4> input_vector_1 =
      input_vector_buffer.Load<vector<uint32_t, 4> >(0);
    const uint is_input_unsigned_1 = 1;

 // expected-no-diagnostics@+1 
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned_1, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

    vector<float16_t, 4> input_vector_2 =
      input_vector_buffer.Load<vector<float16_t, 4> >(0);
    const uint is_input_unsigned_2 = 0;

 // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_2,
                      is_input_unsigned_2, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check valid input vector packed types
void test_valid_input_vector_packed_types() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

 const uint input_interpretation_0 = DataType::DATA_TYPE_UINT8_T4_PACKED;
 vector<uint32_t, 1> input_vector_0 =
     input_vector_buffer.Load<vector<uint32_t, 1> >(0);
 const uint is_input_unsigned_0 = 1;

 // expected-no-diagnostics@+1
 __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0, 
                     is_input_unsigned_0, input_interpretation_0, matrix_buffer,
                     matrix_offset, matrix_interpretation, matrix_dimM,
                     matrix_dimK, matrix_layout, matrix_is_transposed,
                     matrix_stride);

 const uint input_interpretation_1 = DataType::DATA_TYPE_SINT8_T4_PACKED;
 vector<uint32_t, 1> input_vector_1 =
     input_vector_buffer.Load<vector<uint32_t, 1> >(0);
 const uint is_input_unsigned_1 = 1;

 // expected-no-diagnostics@+1  
 __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                     is_input_unsigned_1, input_interpretation_1, matrix_buffer,
                     matrix_offset, matrix_interpretation, matrix_dimM,
                     matrix_dimK, matrix_layout, matrix_is_transposed,
                     matrix_stride);                  

}

// IsInputUnsigned must be true for packed input vector type
void test_valid_is_input_unsigned_packed_input_vector_type() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;  
  const uint matrix_stride = 64;

  const uint input_interpretation_0 = DataType::DATA_TYPE_UINT8_T4_PACKED;  
  vector<uint, 1> input_vector_0 = 
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint is_input_unsigned_0 = 1;

  // expected-no-diagnostics@+2
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned_0, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,  
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_1 = DataType::DATA_TYPE_SINT8_T4_PACKED;
  vector<uint, 1> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint is_input_unsigned_1 = 1;
  
  // expected-no-diagnostics@+2
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned_1, input_interpretation_1, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check packed input vector dimension
void test_valid_packed_input_vector_dimension() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint is_input_unsigned = 1;
  const uint input_interpretation = DataType::DATA_TYPE_UINT8_T4_PACKED;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_UINT8;
  const uint matrix_dimM = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_MUL_OPTIMAL;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 0;

  vector<uint, 1> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint matrix_dimK_0 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_0, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 2> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 2> >(0);
  const uint matrix_dimK_1 = 7;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned, input_interpretation, matrix_buffer, 
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_1, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if Matrix M dimension is less than Max
void test_valid_matrix_M_dimension_less_than_Max() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint is_input_unsigned = 1;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = matrix_dimK * 4;

  vector<uint, 4> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint input_interpretation_0 = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM_0 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM_0,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 1> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint input_interpretation_1 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  const uint matrix_dimM_1 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned, input_interpretation_1, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM_1,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if Matrix K dimension is less than Max in unpacked input vector case
void test_valid_matrix_K_dimension_less_than_Max_unpacked_input_vector() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint is_input_unsigned = 1;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  vector<uint, 4> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint input_interpretation_0 = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimK_0 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_0, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 4> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint input_interpretation_1 = DataType::DATA_TYPE_UINT8;
  const uint matrix_dimK_1 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1, 
                      is_input_unsigned, input_interpretation_1, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_1, matrix_layout, matrix_is_transposed,
                      matrix_stride);

}

// Check if Matrix M dimension is less than Max in packed input vector case
void test_valid_matrix_M_dimension_less_than_Max_packed_input_vector() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint is_input_unsigned = 1;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  vector<uint, 1024> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 1024> >(0);
  const uint input_interpretation_0 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  const uint matrix_dimK_0 = 4096;

  // expected-no-diagnostics@+1
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_0, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}
