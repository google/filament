// RUN: %dxc -T lib_6_9 %s | FileCheck %s
 
ByteAddressBuffer matrix_buffer;
ByteAddressBuffer bias_buffer;
RWByteAddressBuffer rw_matrix_buffer;
ByteAddressBuffer input_vector_buffer;
RWByteAddressBuffer output_vector_buffer;

void UseCoopVec() {
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

    __builtin_MatVecMul(output_vector, is_output_unsigned, input_vector,
      is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
      matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
      matrix_is_transposed, matrix_stride);
    output_vector_buffer.Store(0, output_vector);

    const uint bias_offset = 0;
    const uint bias_interpretation = 9; /*F32*/

    __builtin_MatVecMulAdd(output_vector, is_output_unsigned, input_vector,
      is_input_unsigned, input_interpretation, matrix_buffer, matrix_offset,
      matrix_interpretation, matrix_dimM, matrix_dimK, matrix_layout,
      matrix_is_transposed, matrix_stride, bias_buffer, bias_offset,
      bias_interpretation);
    output_vector_buffer.Store(1024, output_vector);

    vector<uint, 8> input_vector1;
    vector<uint, 8> input_vector2;
    const uint opa_matrix_offset = 0;
    const uint opa_matrix_interpretation = 5; /*U32*/
    const uint opa_matrix_layout = 3; /*OuterProductOptimal*/
    const uint opa_matrix_stride = 0;

    __builtin_OuterProductAccumulate(input_vector1, input_vector2,
      rw_matrix_buffer, opa_matrix_offset, opa_matrix_interpretation,
      opa_matrix_layout, opa_matrix_stride);

    const uint va_matrix_offset = 0;

     __builtin_VectorAccumulate(input_vector1, rw_matrix_buffer,
       va_matrix_offset);
}

// CHECK: define void @ps_main()
// CHECK: call <4 x float> @dx.op.matVecMul
// CHECK: call <4 x float> @dx.op.matVecMulAdd
// CHECK: call void @dx.op.outerProductAccumulate
// CHECK: call void @dx.op.vectorAccumulate

[Shader("pixel")]
void ps_main()
{
    UseCoopVec();
}

// CHECK: define void @cs_main()
// CHECK: call <4 x float> @dx.op.matVecMul
// CHECK: call <4 x float> @dx.op.matVecMulAdd
// CHECK: call void @dx.op.outerProductAccumulate
// CHECK: call void @dx.op.vectorAccumulate

[Shader("compute")]
[NumThreads(1,1,1)]
void cs_main()
{
    UseCoopVec();
}

// CHECK: define void @vs_main()
// CHECK: call <4 x float> @dx.op.matVecMul
// CHECK: call <4 x float> @dx.op.matVecMulAdd
// CHECK: call void @dx.op.outerProductAccumulate
// CHECK: call void @dx.op.vectorAccumulate

[Shader("vertex")]
void vs_main()
{
    UseCoopVec();
}

struct MyRecord{
    uint a;
};

// CHECK: define void @ns_main()
// CHECK: call <4 x float> @dx.op.matVecMul
// CHECK: call <4 x float> @dx.op.matVecMulAdd
// CHECK: call void @dx.op.outerProductAccumulate
// CHECK: call void @dx.op.vectorAccumulate

[Shader("node")]
[NodeLaunch("thread")]
void ns_main(ThreadNodeInputRecord<MyRecord> input)
{
    UseCoopVec();
}

// Vertex shader output structure
struct VS_OUT {
    float3 Color : COLOR0;
};

// Geometry shader output structure
struct GS_OUT {
    float3 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

// CHECK: define void @gs_main()
// CHECK:  call <4 x float> @dx.op.matVecMul
// CHECK: call <4 x float> @dx.op.matVecMulAdd
// CHECK: call void @dx.op.outerProductAccumulate
// CHECK: call void @dx.op.vectorAccumulate

[shader("geometry")]
[maxvertexcount(3)]
void gs_main(point VS_OUT input[1], 
    inout TriangleStream<GS_OUT> OutputStream)
{
    UseCoopVec();
}
