// RUN: %dxc -fcgl -T cs_6_9 -E cs_main %s | FileCheck %s

ByteAddressBuffer input_vector_buffer;
ByteAddressBuffer opa_input_buffer;
ByteAddressBuffer matrix_buffer;
ByteAddressBuffer bias_buffer;
RWByteAddressBuffer rw_matrix_buffer;
RWByteAddressBuffer output_vector_buffer;

[Shader("compute")]
[NumThreads(1,1,1)]
void cs_main()
{    
    vector<float, 4> output_vector;
    static const uint is_output_unsigned = 0;
    
    vector<float, 4> input_vector = input_vector_buffer.Load<vector<float, 4> >(0);
    const uint is_input_unsigned = 0;
    const uint input_interpretation = 9; /*F32*/
    
    const uint matrix_offset = 0;
    const uint matrix_interpretation = 9; /*F32*/
    const uint matrix_dimM = 4;
    const uint matrix_dimK = 4;
    const uint matrix_layout = 0; /*RowMajor*/
    const bool matrix_is_transposed = false; 
    const uint matrix_stride = 64;

    // CHECK: %[[MLD0:[^ ]+]] = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?matrix_buffer@@3UByteAddressBuffer@@A"
    // CHECK: %[[MCH0:[^ ]+]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %[[MLD0]])
    // CHECK: %[[MAH0:[^ ]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %[[MCH0]], %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer undef)
    // CHECK: call void @"dx.hl.op..void (i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32)"(i32 390, <4 x float>* %{{[^ ]+}}, i1 false, <4 x float> %{{[^ ]+}}, i1 false, i32 9, %dx.types.Handle %[[MAH0]], i32 0, i32 9, i32 4, i32 4, i32 0, i1 false, i32 64)
    __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
      is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
      matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
      matrix_is_transposed, matrix_stride);
    output_vector_buffer.Store(0, output_vector);

    const uint bias_offset = 0;
    const uint bias_interpretation = 9; /*F32*/

    // CHECK: %[[MLD1:[^ ]+]] = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?matrix_buffer@@3UByteAddressBuffer@@A"
    // CHECK: %[[MCH1:[^ ]+]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %[[MLD1]])
    // CHECK: %[[MAH1:[^ ]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %[[MCH1]], %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer undef)
    // CHECK-NEXT: %[[BLD1:[^ ]+]] = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?bias_buffer@@3UByteAddressBuffer@@A"
    // CHECK-NEXT: %[[BCH1:[^ ]+]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %[[BLD1]])
    // CHECK-NEXT: %[[BAH1:[^ ]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %[[BCH1]], %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer undef)
    // CHECK-NEXT: call void @"dx.hl.op..void (i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32, %dx.types.Handle, i32, i32)"(i32 391, <4 x float>* %{{[^ ]+}}, i1 false, <4 x float> %{{[^ ]+}}, i1 false, i32 9, %dx.types.Handle %[[MAH1]], i32 0, i32 9, i32 4, i32 4, i32 0, i1 false, i32 64, %dx.types.Handle %[[BAH1]], i32 0, i32 9)
    __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector,
      is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
      matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
      matrix_is_transposed, matrix_stride, bias_buffer, bias_offset,
      bias_interpretation);
    output_vector_buffer.Store(1024, output_vector);

    vector<uint, 8> input_vector1 = opa_input_buffer.Load<vector<uint, 8> >(0);
    vector<uint, 8> input_vector2 = opa_input_buffer.Load<vector<uint, 8> >(128);
    const uint opa_matrix_offset = 0;
    const uint opa_matrix_interpretation = 5; /*U32*/
    const uint opa_matrix_layout = 3; /*OuterProductOptimal*/
    const uint opa_matrix_stride = 0;

    // CHECK: %[[MLD2:[^ ]+]] = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?rw_matrix_buffer@@3URWByteAddressBuffer@@A"
    // CHECK: %[[MCH2:[^ ]+]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %[[MLD2]])
    // CHECK: %[[MAH2:[^ ]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %[[MCH2]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
    // CHECK: call void @"dx.hl.op..void (i32, <8 x i32>, <8 x i32>, %dx.types.Handle, i32, i32, i32, i32)"(i32 392, <8 x i32> %{{[^ ]+}}, <8 x i32> %{{[^ ]+}}, %dx.types.Handle %[[MAH2]], i32 0, i32 5, i32 3, i32 0)
    __builtin_OuterProductAccumulate(input_vector1, input_vector2,
      rw_matrix_buffer, opa_matrix_offset, opa_matrix_interpretation,
      opa_matrix_layout, opa_matrix_stride);

    const uint va_matrix_offset = 0;

    // CHECK: %[[MLD3:[^ ]+]] = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?rw_matrix_buffer@@3URWByteAddressBuffer@@A"
    // CHECK: %[[MCH3:[^ ]+]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %[[MLD3]])
    // CHECK: %[[MAH3:[^ ]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %[[MCH3]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
    // CHECK: call void @"dx.hl.op..void (i32, <8 x i32>, %dx.types.Handle, i32)"(i32 393, <8 x i32> %{{[^ ]+}}, %dx.types.Handle %[[MAH3]], i32 0)
    __builtin_VectorAccumulate(input_vector1, rw_matrix_buffer,
      va_matrix_offset); 
}