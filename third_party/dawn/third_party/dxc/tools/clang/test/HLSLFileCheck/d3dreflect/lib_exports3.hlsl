// RUN: %dxc -auto-binding-space 13 -exports PSMain,PSMain_Clone1,PSMain_Clone2=PSMain -T lib_6_3 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

Buffer<int> T0;

Texture2D<float4> T1;

struct Foo { uint u; float f; };
StructuredBuffer<Foo> T2;

[shader("vertex")]
float4 VSMain(int3 coord : COORD) : SV_Position {
  return T1.Load(coord);
}

[shader("pixel")]
float4 PSMain(int idx : INDEX) : SV_Target {
  return T2[T0.Load(idx)].f;
}

// CHECK: DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:   StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:   IndexTable (size = {{[0-9]+}} bytes)
// CHECK:   RawBytes (size = {{[0-9]+}} bytes)
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) ResourceTable[2] = {
// CHECK:     <0:RuntimeDataResourceInfo> = {
// CHECK:       Class: SRV
// CHECK:       Kind: TypedBuffer
// CHECK:       ID: 0
// CHECK:       Space: 13
// CHECK:       LowerBound: 0
// CHECK:       UpperBound: 0
// CHECK:       Name: "T0"
// CHECK:       Flags: 0 (None)
// CHECK:     }
// CHECK:     <1:RuntimeDataResourceInfo> = {
// CHECK:       Class: SRV
// CHECK:       Kind: StructuredBuffer
// CHECK:       ID: 1
// CHECK:       Space: 13
// CHECK:       LowerBound: 1
// CHECK:       UpperBound: 1
// CHECK:       Name: "T2"
// CHECK:       Flags: 0 (None)
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[6] = {
// CHECK:     <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?PSMain{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "PSMain"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:         [1]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Library
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// CHECK:       MinShaderTarget: 0x60060
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:     }
// CHECK:     <1:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?PSMain_Clone1{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "PSMain_Clone1"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:         [1]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Library
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// CHECK:       MinShaderTarget: 0x60060
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:     }
// CHECK:     <2:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?PSMain_Clone2{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "PSMain_Clone2"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:         [1]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Library
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// CHECK:       MinShaderTarget: 0x60060
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:     }
// CHECK:     <3:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "PSMain"
// CHECK:       UnmangledName: "PSMain"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:         [1]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Pixel
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Pixel)
// CHECK:       MinShaderTarget: 0x60
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:       PS: <0:PSInfo> = {
// CHECK:         SigInputElements: <3:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:           [0]: <0:SignatureElement>
// CHECK:         }
// CHECK:         SigOutputElements: <5:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:           [0]: <1:SignatureElement>
// CHECK:         }
// CHECK:       }
// CHECK:     }
// CHECK:     <4:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "PSMain_Clone1"
// CHECK:       UnmangledName: "PSMain_Clone1"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:         [1]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Pixel
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Pixel)
// CHECK:       MinShaderTarget: 0x60
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:       PS: <0:PSInfo>
// CHECK:     }
// CHECK:     <5:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "PSMain_Clone2"
// CHECK:       UnmangledName: "PSMain_Clone2"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:         [1]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Pixel
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Pixel)
// CHECK:       MinShaderTarget: 0x60
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:       PS: <0:PSInfo>
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) SignatureElementTable[2] = {
// CHECK:     <0:SignatureElement> = {
// CHECK:       SemanticName: "INDEX"
// CHECK:       SemanticIndices: <3:array[1]> = { 0 }
// CHECK:       SemanticKind: Arbitrary
// CHECK:       ComponentType: I32
// CHECK:       InterpolationMode: Constant
// CHECK:       StartRow: 0
// CHECK:       ColsAndStream: 0
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:     <1:SignatureElement> = {
// CHECK:       SemanticName: "SV_Target"
// CHECK:       SemanticIndices: <3:array[1]> = { 0 }
// CHECK:       SemanticKind: Target
// CHECK:       ComponentType: F32
// CHECK:       InterpolationMode: Undefined
// CHECK:       StartRow: 0
// CHECK:       ColsAndStream: 3
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) PSInfoTable[1] = {
// CHECK:     <0:PSInfo> = {
// CHECK:       SigInputElements: <3:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:         [0]: <0:SignatureElement>
// CHECK:       }
// CHECK:       SigOutputElements: <5:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:         [0]: <1:SignatureElement>
// CHECK:       }
// CHECK:     }
// CHECK:   }

// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     FunctionCount: 6
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?PSMain{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?PSMain_Clone1{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?PSMain_Clone2{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: PSMain
// CHECK:       Shader Version: Pixel 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: PSMain_Clone1
// CHECK:       Shader Version: Pixel 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: PSMain_Clone2
// CHECK:       Shader Version: Pixel 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
