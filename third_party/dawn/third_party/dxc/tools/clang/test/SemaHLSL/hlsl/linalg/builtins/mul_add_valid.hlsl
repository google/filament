// RUN: %dxc -I %hlsl_headers -T lib_6_9 %s

#include <dx/linalg.h>

using namespace dx::linalg;

ByteAddressBuffer input_vector_buffer;
ByteAddressBuffer matrix_buffer;
ByteAddressBuffer bias_buffer;
RWByteAddressBuffer output_vector_buffer;
ByteAddressBuffer constants_buffer;

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
  const uint bias_offset = 0;
  const uint bias_interpretation = DataType::DATA_TYPE_FLOAT32;

 const uint input_interpretation_0 = DataType::DATA_TYPE_UINT8_T4_PACKED;
 vector<uint32_t, 1> input_vector_0 =
     input_vector_buffer.Load<vector<uint32_t, 4> >(0);
 const uint is_input_unsigned_0 = 1;

 // expected-no-diagnostics@+1
 __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_0, 
                       is_input_unsigned_0, input_interpretation_0, matrix_buffer,
                       matrix_offset, matrix_interpretation, matrix_dimM,
                       matrix_dimK, matrix_layout, matrix_is_transposed,
                       matrix_stride, bias_buffer, bias_offset, bias_interpretation);

 const uint input_interpretation_1 = DataType::DATA_TYPE_SINT8_T4_PACKED;
 vector<uint32_t, 1> input_vector_1 =
     input_vector_buffer.Load<vector<uint32_t, 1> >(0);
 const uint is_input_unsigned_1 = 1;

 // expected-no-diagnostics@+1  
 __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_1,
                       is_input_unsigned_1, input_interpretation_1, matrix_buffer,
                       matrix_offset, matrix_interpretation, matrix_dimM,
                       matrix_dimK, matrix_layout, matrix_is_transposed,
                       matrix_stride, bias_buffer, bias_offset, bias_interpretation);                  

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
  const uint bias_offset = 0;
  const uint bias_interpretation = DataType::DATA_TYPE_FLOAT32;

  const uint input_interpretation_0 = DataType::DATA_TYPE_UINT8_T4_PACKED;  
  vector<uint, 1> input_vector_0 = 
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint is_input_unsigned_0 = 1;

  // expected-no-diagnostics@+2
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_0,
                        is_input_unsigned_0, input_interpretation_0, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM,  
                        matrix_dimK, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);

  const uint input_interpretation_1 = DataType::DATA_TYPE_SINT8_T4_PACKED;
  vector<uint, 1> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint is_input_unsigned_1 = 1;
  
  // expected-no-diagnostics@+2
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_1,
                        is_input_unsigned_1, input_interpretation_1, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM,
                        matrix_dimK, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);
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
  const uint bias_offset = 0;
  const uint bias_interpretation = DataType::DATA_TYPE_UINT32;

  vector<uint, 1> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint matrix_dimK_0 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_0,
                        is_input_unsigned, input_interpretation, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM,
                        matrix_dimK_0, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);

  vector<uint, 2> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 2> >(0);
  const uint matrix_dimK_1 = 7;

  // expected-no-diagnostics@+1
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_1,
                        is_input_unsigned, input_interpretation, matrix_buffer, 
                        matrix_offset, matrix_interpretation, matrix_dimM,
                        matrix_dimK_1, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);
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
  const uint bias_offset = 0;
  const uint bias_interpretation = DataType::DATA_TYPE_FLOAT32;

  vector<uint, 4> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint input_interpretation_0 = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM_0 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_0,
                        is_input_unsigned, input_interpretation_0, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM_0,
                        matrix_dimK, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);

  vector<uint, 1> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint input_interpretation_1 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  const uint matrix_dimM_1 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_1,
                        is_input_unsigned, input_interpretation_1, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM_1,
                        matrix_dimK, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);
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
  const uint bias_offset = 0;
  const uint bias_interpretation = DataType::DATA_TYPE_FLOAT32;

  vector<uint, 4> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint input_interpretation_0 = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimK_0 = 4;

  // expected-no-diagnostics@+1
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_0,
                        is_input_unsigned, input_interpretation_0, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM,
                        matrix_dimK_0, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);

  vector<uint, 4> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint input_interpretation_1 = DataType::DATA_TYPE_UINT8;
  const uint matrix_dimK_1 = 4;
  // expected-no-diagnostics@+1
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_1, 
                        is_input_unsigned, input_interpretation_1, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM,
                        matrix_dimK_1, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);

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
  const uint bias_offset = 0;
  const uint bias_interpretation = DataType::DATA_TYPE_FLOAT32;

  vector<uint, 1024> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 1024> >(0);
  const uint input_interpretation_0 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  const uint matrix_dimK_0 = 4096;

  // expected-no-diagnostics@+1
  __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector_0,
                        is_input_unsigned, input_interpretation_0, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM,
                        matrix_dimK_0, matrix_layout, matrix_is_transposed,
                        matrix_stride, bias_buffer, bias_offset, bias_interpretation);
}



