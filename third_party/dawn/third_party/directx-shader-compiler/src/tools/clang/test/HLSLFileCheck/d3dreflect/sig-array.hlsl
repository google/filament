// RUN: %dxc -E VSMain -T vs_6_0 %s | %D3DReflect %s | FileCheck %s

// CHECK: ID3D12ShaderReflection:
// CHECK:   D3D12_SHADER_DESC:
// CHECK:     Shader Version: Vertex 6.0
// CHECK:     ConstantBuffers: 0
// CHECK:     BoundResources: 0
// CHECK:     InputParameters: 11
// CHECK:     OutputParameters: 11
// CHECK:   InputParameter Elements: 11
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 0
// CHECK-NEXT:       Register: 0
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: x---
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 1
// CHECK-NEXT:       Register: 1
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: x---
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 2
// CHECK-NEXT:       Register: 2
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: x---
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 3
// CHECK-NEXT:       Register: 3
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: x---
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: F SemanticIndex: 0
// CHECK-NEXT:       Register: 4
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: x---
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: G SemanticIndex: 0
// CHECK-NEXT:       Register: 5
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: x---
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 0
// CHECK-NEXT:       Register: 6
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: xy--
// CHECK-NEXT:       ReadWriteMask: xy--
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 1
// CHECK-NEXT:       Register: 7
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: xy--
// CHECK-NEXT:       ReadWriteMask: xy--
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 2
// CHECK-NEXT:       Register: 8
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: xy--
// CHECK-NEXT:       ReadWriteMask: xy--
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 3
// CHECK-NEXT:       Register: 9
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: xy--
// CHECK-NEXT:       ReadWriteMask: xy--
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: SV_Position SemanticIndex: 0
// CHECK-NEXT:       Register: 10
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: xyzw
// CHECK-NEXT:       ReadWriteMask: xyzw
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:   OutputParameter Elements: 11
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 0
// CHECK-NEXT:       Register: 0
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: -yzw
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 1
// CHECK-NEXT:       Register: 1
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: -yzw
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 2
// CHECK-NEXT:       Register: 2
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: -yzw
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ARRAY SemanticIndex: 3
// CHECK-NEXT:       Register: 3
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: x---
// CHECK-NEXT:       ReadWriteMask: -yzw
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: F SemanticIndex: 0
// CHECK-NEXT:       Register: 0
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: -y--
// CHECK-NEXT:       ReadWriteMask: x-zw
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: G SemanticIndex: 0
// CHECK-NEXT:       Register: 0
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: --z-
// CHECK-NEXT:       ReadWriteMask: xy-w
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 0
// CHECK-NEXT:       Register: 1
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: -yz-
// CHECK-NEXT:       ReadWriteMask: x--w
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 1
// CHECK-NEXT:       Register: 2
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: -yz-
// CHECK-NEXT:       ReadWriteMask: x--w
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 2
// CHECK-NEXT:       Register: 3
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: -yz-
// CHECK-NEXT:       ReadWriteMask: x--w
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MATRIX_ARRAY SemanticIndex: 3
// CHECK-NEXT:       Register: 4
// CHECK-NEXT:       SystemValueType: D3D_NAME_UNDEFINED
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: -yz-
// CHECK-NEXT:       ReadWriteMask: x--w
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT
// CHECK-NEXT:     D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: SV_POSITION SemanticIndex: 0
// CHECK-NEXT:       Register: 5
// CHECK-NEXT:       SystemValueType: D3D_NAME_POSITION
// CHECK-NEXT:       ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// CHECK-NEXT:       Mask: xyzw
// CHECK-NEXT:       ReadWriteMask: ----
// CHECK-NEXT:       Stream: 0
// CHECK-NEXT:       MinPrecision: D3D_MIN_PRECISION_DEFAULT

struct IO {
   float arr[4] : ARRAY;
   float f : F,
         g : G;
   float2x2 mat_arr[2] : MATRIX_ARRAY;
   float4 pos : SV_Position;
};

void VSMain(inout IO io) {
}
