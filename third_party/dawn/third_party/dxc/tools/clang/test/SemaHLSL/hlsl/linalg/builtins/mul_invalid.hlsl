// RUN: %dxc -I %hlsl_headers -T lib_6_9 -enable-16bit-types %s -verify

#include <dx/linalg.h>

using namespace dx::linalg;

ByteAddressBuffer input_vector_buffer;
ByteAddressBuffer matrix_buffer;
RWByteAddressBuffer output_vector_buffer;
ByteAddressBuffer constants_buffer;

// Output vector, isUnsigned mismatch
void test_invalid_output_vector_type() {

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

  vector<uint, 4> output_vector_0;
  const uint is_output_unsigned_0 = 0;

  // expected-error@+1 {{IsOuputUnsigned must be true for vector of unsigned integer type}}
  __builtin_MatVecMul(output_vector_0, is_output_unsigned_0, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<int32_t, 4> output_vector_1;
  const uint is_output_unsigned_1 = 1;

  // expected-error@+1 {{IsOuputUnsigned must be false for vector of signed integer type}}
  __builtin_MatVecMul(output_vector_1, is_output_unsigned_1, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<float, 4> output_vector_2;
  const uint is_output_unsigned_2 = 1;

  // expected-error@+1 {{IsOuputUnsigned must be false for vector of floating point type}}
  __builtin_MatVecMul(output_vector_2, is_output_unsigned_2, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// IsOutputUnsigned is not a constant parameter
void test_invalid_is_output_unsigned_non_const() {

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

  const uint is_output_unsigned_0 = constants_buffer.Load<uint>(0);

  // expected-error@+1 {{expression is not an integer constant expression}}
  __builtin_MatVecMul(output_vector_0, is_output_unsigned_0, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Input vector is incorrect type - 64 bit types
void test_invalid_input_vector_type() {

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

    vector<int64_t, 4> input_vector_0 =
      input_vector_buffer.Load<vector<int64_t, 4> >(0);
    const uint is_input_unsigned_0 = 0;

// expected-error@+2 {{no matching function for call to '__builtin_MatVecMul'}}
// expected-note@+1 {{candidate function not viable: no known conversion from 'vector<int64_t, 4>' to 'vector<float, 4>' for 3rd argument}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned_0, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

    vector<uint64_t, 4> input_vector_1 =
      input_vector_buffer.Load<vector<uint64_t, 4> >(0);
    const uint is_input_unsigned_1 = 1;

// expected-error@+2 {{no matching function for call to '__builtin_MatVecMul'}}
// expected-note@+1 {{candidate function not viable: no known conversion from 'vector<uint64_t, 4>' to 'vector<float, 4>' for 3rd argument}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned_1, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

    vector<float64_t, 4> input_vector_2 =
      input_vector_buffer.Load<vector<float64_t, 4> >(0);
    const uint is_input_unsigned_2 = 0;

// expected-error@+2 {{no matching function for call to '__builtin_MatVecMul'}}
// expected-note@+1 {{candidate function not viable: no known conversion from 'vector<float64_t, 4>' to 'vector<float, 4>' for 3rd argument}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_2,
                      is_input_unsigned_2, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Input vector is incorrect type for packed InputInterpretation
void test_invalid_input_vector_type_packed_input_interpretation() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint input_interpretation_0 = DataType::DATA_TYPE_SINT8_T4_PACKED;
  vector<int16_t, 2> input_vector_0 =
      input_vector_buffer.Load<vector<int16_t, 2> >(0);
  const uint is_input_unsigned_0 = 1;

  // expected-error@+1 {{packed input vector type must be a 32-bit unsigned int type in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned_0, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_1 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  vector<uint16_t, 2> input_vector_1 =
      input_vector_buffer.Load<vector<uint16_t, 2> >(0);
  const uint is_input_unsigned_1 = 0;

  // expected-error@+1 {{packed input vector type must be a 32-bit unsigned int type in linalg mul/muladd operations}} 
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned_1, input_interpretation_1, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_2 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  vector<int32_t, 1> input_vector_2 =
      input_vector_buffer.Load<vector<int32_t, 1> >(0);
  const uint is_input_unsigned_2 = 1;
  
  // expected-error@+1 {{packed input vector type must be a 32-bit unsigned int type in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_2,
                      is_input_unsigned_2, input_interpretation_2, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_3 = DataType::DATA_TYPE_SINT8_T4_PACKED;
  vector<int32_t, 1> input_vector_3 =
      input_vector_buffer.Load<vector<int32_t, 1> >(0);
  const uint is_input_unsigned_3 = 0;

  // expected-error@+1 {{packed input vector type must be a 32-bit unsigned int type in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_3,
                      is_input_unsigned_3, input_interpretation_3, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_4 = DataType::DATA_TYPE_SINT8_T4_PACKED;
  vector<float, 1> input_vector_4 =
      input_vector_buffer.Load<vector<float, 1> >(0);
  const uint is_input_unsigned_4 = 0;

  // expected-error@+1 {{packed input vector type must be a 32-bit unsigned int type in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_4, 
                      is_input_unsigned_4, input_interpretation_4, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// IsInputUnsigned must be true for packed input vector type
void test_invalid_is_input_unsigned_packed_input_vector_type() {

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
  const uint is_input_unsigned_0 = 0;

  // expected-error@+2 {{IsInputUnsigned must be true for packed input interpretations in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned_0, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,  
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_1 = DataType::DATA_TYPE_SINT8_T4_PACKED;
  vector<uint, 1> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint is_input_unsigned_1 = 0;
  
  // expected-error@+2 {{IsInputUnsigned must be true for packed input interpretations in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned_1, input_interpretation_1, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check packed input vector dimension
void test_invalid_packed_input_vector_dimension() {

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

  vector<uint, 4> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint matrix_dimK_0 = 4;

  // expected-error@+1 {{packed input vector length must be the smallest number that can hold matrix dim K values of the packed(smaller) type in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_0, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 1> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint matrix_dimK_1 = 7;

  // expected-error@+1 {{packed input vector length must be the smallest number that can hold matrix dim K values of the packed(smaller) type in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned, input_interpretation, matrix_buffer, 
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_1, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 3> input_vector_2 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint matrix_dimK_2 = 7;

  // expected-error@+1 {{packed input vector length must be the smallest number that can hold matrix dim K values of the packed(smaller) type in linalg mul/muladd operations}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_2,
                      is_input_unsigned, input_interpretation, matrix_buffer, 
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_2, matrix_layout, matrix_is_transposed,
                      matrix_stride);

}

// Input vector type/isInputUnsigned mismatch
void test_invalid_input_vector_type_mismatch() {

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

  vector<uint, 4> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 4> >(0);    
  const uint is_input_unsigned_0 = 0;

  // expected-error@+2 {{IsInputUnsigned must be true for vector of unsigned integer type}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned_0, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<int32_t, 4> input_vector_1 =
      input_vector_buffer.Load<vector<int32_t, 4> >(0);
  const uint is_input_unsigned_1 = 1;

  // expected-error@+2 {{IsInputUnsigned must be false for vector of signed integer type}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned_1, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<float16_t, 4> input_vector_2 =
      input_vector_buffer.Load<vector<float16_t, 4> >(0);
  const uint is_input_unsigned_2 = 1;

  // expected-error@+2 {{IsInputUnsigned must be false for vector of floating point type}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_2,
                      is_input_unsigned_2, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

//  Check is Matrix M dimension is a constant parameter
void test_invalid_matrix_M_dimension() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64; 

  const uint matrix_dimM = constants_buffer.Load<uint>(0);   
  
  // expected-error@+3 {{expression is not an integer constant expression}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

//  Check is Matrix K dimension is a constant parameter
void test_invalid_matrix_K_dimension() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0; 
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint matrix_dimK = constants_buffer.Load<uint>(0);
  
  // expected-error@+4 {{expression is not an integer constant expression}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check is Matrix M dimension is non-zero
void test_invalid_matrix_M_dimension_non_zero() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint matrix_dimM = 0;
  // expected-error@+3 {{matrix dimension must be greater than 0}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check is Matrix K dimension is non-zero
void test_invalid_matrix_K_dimension_non_zero() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint matrix_dimK = 0;
  // expected-error@+4 {{matrix dimension must be greater than 0}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if Matrix M dimension is less than Max
void test_invalid_matrix_M_dimension_less_than_Max() {

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
  const uint matrix_dimM_0 = 1025;

  // expected-error@+3 {{matrix dimension M must be less than 1024, in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM_0,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 1> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 1> >(0);
  const uint input_interpretation_1 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  const uint matrix_dimM_1 = 4097;

  // expected-error@+3 {{matrix dimension M must be less than 1024, in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1,
                      is_input_unsigned, input_interpretation_1, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM_1,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if Matrix K dimension is less than Max in unpacked input vector case
void test_invalid_matrix_K_dimension_less_than_Max_unpacked_input_vector() {

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
  const uint matrix_dimK_0 = 1025;

  // expected-error@+4 {{matrix dimension K when using unpacked input vectors must be less than 1024, in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_0, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 4> input_vector_1 =
      input_vector_buffer.Load<vector<uint, 4> >(0);
  const uint input_interpretation_1 = DataType::DATA_TYPE_UINT8;
  const uint matrix_dimK_1 = 4096;
  // expected-error@+4 {{matrix dimension K when using unpacked input vectors must be less than 1024, in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_1, 
                      is_input_unsigned, input_interpretation_1, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_1, matrix_layout, matrix_is_transposed,
                      matrix_stride);

}

// Check if Matrix M dimension is less than Max in packed input vector case
void test_invalid_matrix_M_dimension_less_than_Max_packed_input_vector() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  const uint is_input_unsigned = 1;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 1024;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 4096;

  vector<uint, 1024> input_vector_0 =
      input_vector_buffer.Load<vector<uint, 1024> >(0);
  const uint input_interpretation_0 = DataType::DATA_TYPE_UINT8_T4_PACKED;
  const uint matrix_dimK_0 = 4097;

  // expected-error@+4 {{matrix dimension K when using packed input vectors must be less than 4096, in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector_0,
                      is_input_unsigned, input_interpretation_0, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK_0, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

//Check if InputInterpretation is a constant parameter
void test_invalid_input_interpretation_non_const() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint input_interpretation = constants_buffer.Load<uint>(0);

  // expected-error@+2 {{expression is not an integer constant expression}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if InputInterpretation is a valid value
void test_invalid_input_interpretation_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);   
  const uint is_input_unsigned = 0;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint input_interpretation_0 = 0;

  // expected-error@+2 {{0 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_0, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_1 = 1;

  // expected-error@+2 {{1 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_1, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_2 = 6;

  // expected-error@+2 {{6 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_2, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_3 = 7;

  // expected-error@+2 {{7 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_3, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);   

  const uint input_interpretation_4 = 10;

  // expected-error@+2 {{10 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_4, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,    
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_5 = 11;       

  // expected-error@+2 {{11 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_5, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_6 = 12;

  // expected-error@+2 {{12 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_6, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_7 = 13;

  // expected-error@+2 {{13 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_7, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_8 = 14;

  // expected-error@+2 {{14 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_8, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_9 = 15;

  // expected-error@+2 {{15 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_9, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_10 = 16;

  // expected-error@+2 {{16 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_10, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_11 = 23;

  // expected-error@+2 {{23 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_11, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint input_interpretation_12 = 100;

  // expected-error@+2 {{100 is an invalid register interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation_12, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}
// Check if Input and Output vector dimensions are valid -non packed
void test_invalid_input_output_vector_dimensions_non_packed_square_matrix() {

  const uint is_output_unsigned = 1;
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 32;
  const uint matrix_dimK = 32;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  vector<uint, 32> output_vector_0;
  vector<float, 30> input_vector_0 =   
      input_vector_buffer.Load<vector<float, 30> >(0);

  // expected-error@+1 {{unpacked input vector length must be equal to Matrix K dimension in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector_0, is_output_unsigned, input_vector_0,  
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  vector<uint, 30> output_vector_1;
  vector<float, 32> input_vector_1 =   
      input_vector_buffer.Load<vector<float, 32> >(0);

  // expected-error@+1 {{output vector length must be equal to Matrix M dimension in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector_1, is_output_unsigned, input_vector_1,    
                      is_input_unsigned, input_interpretation, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if Input and Output vector dimensions are valid -non packed
void test_invalid_input_output_vector_dimensions_non_packed_rectangle_matrix() {

  const uint is_output_unsigned = 1;
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 16;
  const uint matrix_dimK = 32;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  // Use dimension of Matrix K to trigger error
  vector<uint, 32> output_vector_0;
  vector<float, 32> input_vector_0 =   
      input_vector_buffer.Load<vector<float, 32> >(0);

  // expected-error@+1 {{output vector length must be equal to Matrix M dimension in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector_0, is_output_unsigned, input_vector_0,  
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
 
 // Check off by 1 errors
  vector<uint, 17> output_vector_1;
  vector<float, 16> input_vector_1 =   
      input_vector_buffer.Load<vector<float, 16> >(0);

  // expected-error@+1 {{output vector length must be equal to Matrix M dimension in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector_1, is_output_unsigned, input_vector_1,    
                      is_input_unsigned, input_interpretation, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

 // Check off by 1 errors
 vector<uint, 15> output_vector_2;
 vector<float, 16> input_vector_2 =   
     input_vector_buffer.Load<vector<float, 16> >(0);

 // expected-error@+1 {{output vector length must be equal to Matrix M dimension in a linalg Mul/MulAdd operation}}         
 __builtin_MatVecMul(output_vector_2, is_output_unsigned, input_vector_2,    
                      is_input_unsigned, input_interpretation, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  // Use dimension of Matrix M to trigger error 
  vector<uint, 16> output_vector_3;
  vector<float, 16> input_vector_3 =   
      input_vector_buffer.Load<vector<float, 16> >(0);

  // expected-error@+1 {{unpacked input vector length must be equal to Matrix K dimension in a linalg Mul/MulAdd operation}}
  __builtin_MatVecMul(output_vector_3, is_output_unsigned, input_vector_3,  
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  // Check off by 1 errors
  vector<uint, 16> output_vector_4;
  vector<float, 31> input_vector_4 =   
      input_vector_buffer.Load<vector<float, 31> >(0);

  // expected-error@+1 {{unpacked input vector length must be equal to Matrix K dimension in a linalg Mul/MulAdd operation}}    
  __builtin_MatVecMul(output_vector_4, is_output_unsigned, input_vector_4,  
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  // Check off by 1 errors
  vector<uint, 16> output_vector_5;
  vector<float, 33> input_vector_5 =   
      input_vector_buffer.Load<vector<float, 33> >(0);

  // expected-error@+1 {{unpacked input vector length must be equal to Matrix K dimension in a linalg Mul/MulAdd operation}}    
  __builtin_MatVecMul(output_vector_5, is_output_unsigned, input_vector_5,  
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

    // Swap dimensions to trigger error
    vector<uint, 32> output_vector_6;
    vector<float, 16> input_vector_6 =   
        input_vector_buffer.Load<vector<float, 16> >(0);

    // expected-error@+1 {{output vector length must be equal to Matrix M dimension in a linalg Mul/MulAdd operation}}    
    __builtin_MatVecMul(output_vector_6, is_output_unsigned, input_vector_6,  
                        is_input_unsigned, input_interpretation, matrix_buffer,
                        matrix_offset, matrix_interpretation, matrix_dimM,
                        matrix_dimK, matrix_layout, matrix_is_transposed,
                        matrix_stride);
}

// Check if matrtrix  interpretation is a constant value
void test_invalid_matrix_interpretation_constant_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
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

  const uint matrix_interpretation_0 = constants_buffer.Load<uint>(0);

  // expected-error@+3 {{expression is not an integer constant expression}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_0, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check for invalid matrix interpretation value
void test_invalid_matrix_interpretation_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint matrix_interpretation_0 = 0;

  // expected-error@+3 {{0 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_0, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_1 = 1;

  // expected-error@+3 {{1 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_1, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_2 = 6;

  // expected-error@+3 {{6 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_2, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_3 = 7;

  // expected-error@+3 {{7 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,   
                      matrix_offset, matrix_interpretation_3, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_4 = 10;

  // expected-error@+3 {{10 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_4, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_5 = 11;

  // expected-error@+3 {{11 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_5, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_6 = 12;

  // expected-error@+3 {{12 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_6, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_7 = 13;

  // expected-error@+3 {{13 is an invalid memory interpretation value}} 
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_7, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);       

  const uint matrix_interpretation_8 = 14;

  // expected-error@+3 {{14 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_8, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_9 = 15;

  // expected-error@+3 {{15 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_9, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_10 = 16;

  // expected-error@+3 {{16 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_10, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_11 = 23;
  // expected-error@+3 {{23 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_11, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);

  const uint matrix_interpretation_12 = 100;

  // expected-error@+3 {{100 is an invalid memory interpretation value}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation_12, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if matrix Layout is a constant value
void test_invalid_matrix_layout_constant_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);   
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint matrix_layout = constants_buffer.Load<uint>(0);

  // expected-error@+4 {{expression is not an integer constant expression}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check invalid matrix layout value
void test_invalid_matrix_layout_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const bool matrix_is_transposed = false;
  const uint matrix_stride = 64;

  const uint matrix_layout_0 = 4;

  // expected-error@+4 {{matrix layout 4 is not valid, must be in the range [0, 3]}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout_0, matrix_is_transposed,
                      matrix_stride);
}

// Check if matrix is transposed is a constant value
void test_invalid_matrix_transposed_constant_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const uint matrix_layout = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed = constants_buffer.Load<bool>(0);
  const uint matrix_stride = 64;

  // expected-error@+4 {{expression is not an integer constant expression}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout, matrix_is_transposed,
                      matrix_stride);
}

// Check if invalid matrix transpose value is used
void test_invalid_matrix_transpose_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =   
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;   
  const uint matrix_stride = 64;

  const uint matrix_layout_0 = MatrixLayout::MATRIX_LAYOUT_ROW_MAJOR;
  const bool matrix_is_transposed_0 = true;

  // expected-error@+4 {{RowMajor and ColumnMajor matrices are not transposable}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout_0, matrix_is_transposed_0,
                      matrix_stride);

  const uint matrix_layout_1 = MatrixLayout::MATRIX_LAYOUT_COLUMN_MAJOR;
  const bool matrix_is_transposed_1 = true;

  // expected-error@+4 {{RowMajor and ColumnMajor matrices are not transposable}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout_1, matrix_is_transposed_1,
                      matrix_stride);
}


// Check invalid matrix stride value for optimal matrix layout
void test_invalid_matrix_stride_constant_value() {

  vector<uint, 4> output_vector;
  const uint is_output_unsigned = 1;
  vector<float, 4> input_vector =
      input_vector_buffer.Load<vector<float, 4> >(0);
  const uint is_input_unsigned = 0;
  const uint input_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_offset = 0;
  const uint matrix_interpretation = DataType::DATA_TYPE_FLOAT32;
  const uint matrix_dimM = 4;
  const uint matrix_dimK = 4;
  const bool matrix_is_transposed = false;

  const uint matrix_layout_0 = MatrixLayout::MATRIX_LAYOUT_MUL_OPTIMAL;
  const uint matrix_stride_0 = 64;

  // expected-error@+5 {{for optimal matrix layout, matrix stride must be 0}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout_0, matrix_is_transposed,
                      matrix_stride_0);

  const uint matrix_layout_1 = MatrixLayout::MATRIX_LAYOUT_OUTER_PRODUCT_OPTIMAL;
  const uint matrix_stride_1 = 64;

  // expected-error@+5 {{for optimal matrix layout, matrix stride must be 0}}
  __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
                      is_input_unsigned, input_interpretation, matrix_buffer,   
                      matrix_offset, matrix_interpretation, matrix_dimM,
                      matrix_dimK, matrix_layout_1, matrix_is_transposed,
                      matrix_stride_1);
}
