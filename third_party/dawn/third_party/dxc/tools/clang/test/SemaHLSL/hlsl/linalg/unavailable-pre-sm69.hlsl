// RUN: %dxc -T lib_6_8 %s -verify
 
ByteAddressBuffer matrix_buffer;
ByteAddressBuffer bias_buffer;
RWByteAddressBuffer rw_matrix_buffer;

[Shader("compute")]
[Numthreads(1,1,1)]
void cs_main()
{    
    vector<float, 4> output_vector;
    static const uint is_output_unsigned = 0;
    
    vector<float, 4> input_vector;
    const uint is_input_unsigned = 0;
    const uint input_interpretation = 9; /*F32*/
    
    const uint matrix_offset = 0;
    const uint matrix_interpretation = 9; /*F32*/
    const uint matrix_dimM = 4;
    const uint matrix_dimK = 4;
    const uint matrix_layout = 0; /*RowMajor*/
    const bool matrix_is_transposed = false; 
    const uint matrix_stride = 64;

    //expected-error@+1{{intrinsic __builtin_MatVecMul potentially used by ''cs_main'' requires shader model 6.9 or greater}}
    __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector, 
      is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
      matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
      matrix_is_transposed, matrix_stride); 

    const uint bias_offset = 0;
    const uint bias_interpretation = 9; /*F32*/

    //expected-error@+1{{intrinsic __builtin_MatVecMulAdd potentially used by ''cs_main'' requires shader model 6.9 or greater}}
    __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector,
      is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
      matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
      matrix_is_transposed, matrix_stride, bias_buffer, bias_offset,
      bias_interpretation); 

    vector<uint, 4> input_vector1;
    vector<uint, 4> input_vector2;
    const uint opa_matrix_offset = 0;
    const uint opa_matrix_interpretation = 5; /*U32*/
    const uint opa_matrix_layout = 3; /*OuterProductOptimal*/
    const uint opa_matrix_stride = 0;

    //expected-error@+1{{intrinsic __builtin_OuterProductAccumulate potentially used by ''cs_main'' requires shader model 6.9 or greater}}
    __builtin_OuterProductAccumulate(input_vector1, input_vector2,
      rw_matrix_buffer, opa_matrix_offset, opa_matrix_interpretation,
      opa_matrix_layout, opa_matrix_stride);

    const uint va_matrix_offset = 0;

    //expected-error@+1{{intrinsic __builtin_VectorAccumulate potentially used by ''cs_main'' requires shader model 6.9 or greater}}
    __builtin_VectorAccumulate(input_vector1, rw_matrix_buffer,
      va_matrix_offset);
}
