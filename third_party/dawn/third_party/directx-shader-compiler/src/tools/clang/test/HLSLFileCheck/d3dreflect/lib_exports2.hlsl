// RUN: %dxc -auto-binding-space 13 -exports VSMain;VS_RENAMED=\01?VSMain@@YA?AV?$vector@M$03@@V?$vector@H$02@@@Z;RayGen1,RayGen2=RayGen -T lib_6_3 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

Buffer<int> T0;

Texture2D<float4> T1;

struct Foo { uint u; float f; };
StructuredBuffer<Foo> T2;

RWByteAddressBuffer U0;

[shader("vertex")]
float4 VSMain(int3 coord : COORD) : SV_Position {
  return T1.Load(coord);
}

[shader("pixel")]
float4 PSMain(int idx : INDEX) : SV_Target {
  return T2[T0.Load(idx)].f;
}

[shader("raygeneration")]
void RayGen() {
  uint2 dim = DispatchRaysDimensions();
  uint2 idx = DispatchRaysIndex();
  U0.Store(idx.y * dim.x * 4 + idx.x * 4, idx.x ^ idx.y);
}

// CHECK: DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:   StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:   IndexTable (size = {{[0-9]+}} bytes)
// CHECK:   RawBytes (size = {{[0-9]+}} bytes)
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) ResourceTable[2] = {
// CHECK:     <0:RuntimeDataResourceInfo> = {
// CHECK:       Class: SRV
// CHECK:       Kind: Texture2D
// CHECK:       ID: 0
// CHECK:       Space: 13
// CHECK:       LowerBound: 0
// CHECK:       UpperBound: 0
// CHECK:       Name: "T1"
// CHECK:       Flags: 0 (None)
// CHECK:     }
// CHECK:     <1:RuntimeDataResourceInfo> = {
// CHECK:       Class: UAV
// CHECK:       Kind: RawBuffer
// CHECK:       ID: 0
// CHECK:       Space: 13
// CHECK:       LowerBound: 0
// CHECK:       UpperBound: 0
// CHECK:       Name: "U0"
// CHECK:       Flags: 0 (None)
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[4] = {
// CHECK:     <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?RayGen1{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "RayGen1"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[1]>  = {
// CHECK:         [0]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: RayGeneration
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (RayGeneration)
// CHECK:       MinShaderTarget: 0x70063
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:     }
// CHECK:     <1:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?VS_RENAMED{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "VS_RENAMED"
// CHECK:       Resources: <2:RecordArrayRef<RuntimeDataResourceInfo>[1]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
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
// CHECK:       Name: "\01?RayGen2{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "RayGen2"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[1]>  = {
// CHECK:         [0]: <1:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: RayGeneration
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (RayGeneration)
// CHECK:       MinShaderTarget: 0x70063
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:     }
// CHECK:     <3:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "VSMain"
// CHECK:       UnmangledName: "VSMain"
// CHECK:       Resources: <2:RecordArrayRef<RuntimeDataResourceInfo>[1]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:       }
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Vertex
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Vertex)
// CHECK:       MinShaderTarget: 0x10060
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: (OutputPositionPresent)
// CHECK:       VS: <0:VSInfo>
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) SignatureElementTable[2] = {
// CHECK:     <0:SignatureElement> = {
// CHECK:       SemanticName: "COORD"
// CHECK:       SemanticIndices: <2:array[1]> = { 0 }
// CHECK:       SemanticKind: Arbitrary
// CHECK:       ComponentType: I32
// CHECK:       InterpolationMode: Undefined
// CHECK:       StartRow: 0
// CHECK:       ColsAndStream: 2
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:     <1:SignatureElement> = {
// CHECK:       SemanticName: "SV_Position"
// CHECK:       SemanticIndices: <2:array[1]> = { 0 }
// CHECK:       SemanticKind: Position
// CHECK:       ComponentType: F32
// CHECK:       InterpolationMode: LinearNoperspective
// CHECK:       StartRow: 0
// CHECK:       ColsAndStream: 3
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) VSInfoTable[1] = {
// CHECK:     <0:VSInfo> = {
// CHECK:       SigInputElements: <2:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:         [0]: <0:SignatureElement>
// CHECK:       }
// CHECK:       SigOutputElements: <0:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:         [0]: <1:SignatureElement>
// CHECK:       }
// CHECK:       ViewIDOutputMask: <0:bytes[0]>
// CHECK:     }
// CHECK:   }

// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     FunctionCount: 4
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?RayGen1{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: RayGeneration 6.3
// CHECK:       BoundResources: 1
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: U0
// CHECK:         Type: D3D_SIT_UAV_RWBYTEADDRESS
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 0
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?RayGen2{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: RayGeneration 6.3
// CHECK:       BoundResources: 1
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: U0
// CHECK:         Type: D3D_SIT_UAV_RWBYTEADDRESS
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 0
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?VS_RENAMED{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 1
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T1
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK:         Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: VSMain
// CHECK:       Shader Version: Vertex 6.3
// CHECK:       BoundResources: 1
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T1
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK:         Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
