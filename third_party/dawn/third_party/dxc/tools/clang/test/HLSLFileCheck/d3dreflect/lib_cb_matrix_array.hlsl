// RUN: %dxc -T lib_6_5 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -T lib_6_6 %s | %D3DReflect %s | FileCheck %s

cbuffer SomeBuffer : register(b0)
{
    float4 Color;
    float4 VectorArray[3];
    float FloatArray[16];
    float4x4 MatrixArray[2];
};

RWTexture2D<float4> g_Output;

[shader("raygeneration")]
void RayGenShader()
{
    uint2 LaunchIndex = DispatchRaysIndex().xy;
    uint2 LaunchDim = DispatchRaysDimensions().xy;

    float4x4 m0 = MatrixArray[0];
    float4x4 m1 = MatrixArray[1];
    g_Output[LaunchIndex] = Color * m0[0][0] * m1[0][0] * VectorArray[1] * FloatArray[2];
}

// CHECK: ID3D12LibraryReflection:
// CHECK:     FunctionCount: 1
// CHECK-NEXT:   ID3D12FunctionReflection:
// CHECK-NEXT:     D3D12_FUNCTION_DESC: Name: \01?RayGenShader{{[@$?.A-Za-z0-9_]+}}
// CHECK-NEXT:       Shader Version: RayGeneration
// CHECK:       ConstantBuffers: 1
// CHECK-NEXT:       BoundResources: 2
// CHECK-NEXT:       FunctionParameterCount: 0
// CHECK-NEXT:       HasReturn: FALSE
// CHECK-NEXT:     Constant Buffers:
// CHECK-NEXT:       ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:         D3D12_SHADER_BUFFER_DESC: Name: SomeBuffer
// CHECK-NEXT:           Type: D3D_CT_CBUFFER
// CHECK-NEXT:           Size: 448
// CHECK-NEXT:           uFlags: 0
// CHECK-NEXT:           Num Variables: 4
// CHECK-NEXT:         {
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: Color
// CHECK-NEXT:               Size: 16
// CHECK-NEXT:               StartOffset: 0
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float4
// CHECK-NEXT:                 Class: D3D_SVC_VECTOR
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT
// CHECK-NEXT:                 Elements: 0
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 4
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: SomeBuffer
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: VectorArray
// CHECK-NEXT:               Size: 48
// CHECK-NEXT:               StartOffset: 16
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float4
// CHECK-NEXT:                 Class: D3D_SVC_VECTOR
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT
// CHECK-NEXT:                 Elements: 3
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 4
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: SomeBuffer
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: FloatArray
// CHECK-NEXT:               Size: 244
// CHECK-NEXT:               StartOffset: 64
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float
// CHECK-NEXT:                 Class: D3D_SVC_SCALAR
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT
// CHECK-NEXT:                 Elements: 16
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 1
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: SomeBuffer
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: MatrixArray
// CHECK-NEXT:               Size: 128
// CHECK-NEXT:               StartOffset: 320
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float4x4
// CHECK-NEXT:                 Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT
// CHECK-NEXT:                 Elements: 2
// CHECK-NEXT:                 Rows: 4
// CHECK-NEXT:                 Columns: 4
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: SomeBuffer
