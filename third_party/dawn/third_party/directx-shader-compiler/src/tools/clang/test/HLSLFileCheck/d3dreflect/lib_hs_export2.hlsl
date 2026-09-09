// RUN: %dxc -auto-binding-space 13 -T lib_6_3 -exports HSMain1;HSMain3 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

Buffer<float> T_unused;

struct PSSceneIn
{
  float4 pos  : SV_Position;
  float2 tex  : TEXCOORD0;
  float3 norm : NORMAL;
};

struct HSPerPatchData
{
  float edges[3] : SV_TessFactor;
  float inside   : SV_InsideTessFactor;
};

// Should not be selected, since later candidate function with same name exists.
// If selected, it should fail, since patch size mismatches HS function.
HSPerPatchData HSPerPatchFunc1(
  const InputPatch< PSSceneIn, 16 > points)
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = T_unused.Load(1).x;

  return d;
}

HSPerPatchData HSPerPatchFunc2(
  const InputPatch< PSSceneIn, 4 > points)
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;

  return d;
}


[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc1")]
[outputcontrolpoints(3)]
void HSMain1( const uint id : SV_OutputControlPointID,
              const InputPatch< PSSceneIn, 3 > points )
{
}

[shader("hull")]
[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc2")]
[outputcontrolpoints(4)]
void HSMain2( const uint id : SV_OutputControlPointID,
              const InputPatch< PSSceneIn, 4 > points )
{
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[patchconstantfunc("HSPerPatchFunc1")]
[outputcontrolpoints(3)]
void HSMain3( const uint id : SV_OutputControlPointID,
              const InputPatch< PSSceneIn, 3 > points )
{
}

// actual selected HSPerPatchFunc1 for HSMain1 and HSMain3
HSPerPatchData HSPerPatchFunc1()
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;

  return d;
}

// CHECK: DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:   StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:   IndexTable (size = {{[0-9]+}} bytes)
// CHECK:   RawBytes (size = {{[0-9]+}} bytes)
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[5] = {
// CHECK:     <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?HSMain1{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "HSMain1"
// CHECK:       Resources: <RecordArrayRef<RuntimeDataResourceInfo>[0]> = {}
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
// CHECK:       Name: "\01?HSMain3{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "HSMain3"
// CHECK:       Resources: <RecordArrayRef<RuntimeDataResourceInfo>[0]> = {}
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
// CHECK:       Name: "HSMain1"
// CHECK:       UnmangledName: "HSMain1"
// CHECK:       Resources: <RecordArrayRef<RuntimeDataResourceInfo>[0]> = {}
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Hull
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Hull)
// CHECK:       MinShaderTarget: 0x30060
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:       HS: <0:HSInfo>
// CHECK:     }
// CHECK:     <3:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "HSMain3"
// CHECK:       UnmangledName: "HSMain3"
// CHECK:       Resources: <RecordArrayRef<RuntimeDataResourceInfo>[0]> = {}
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Hull
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Hull)
// CHECK:       MinShaderTarget: 0x30060
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:       HS: <1:HSInfo>
// CHECK:     }
// CHECK:     <4:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?HSPerPatchFunc1{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "HSPerPatchFunc1"
// CHECK:       Resources: <RecordArrayRef<RuntimeDataResourceInfo>[0]> = {}
// CHECK:       FunctionDependencies: <string[0]> = {}
// CHECK:       ShaderKind: Library
// CHECK:       PayloadSizeInBytes: 0
// CHECK:       AttributeSizeInBytes: 0
// CHECK:       FeatureInfo1: 0
// CHECK:       FeatureInfo2: 0
// CHECK:       ShaderStageFlag: (Hull)
// CHECK:       MinShaderTarget: 0x60060
// CHECK:       MinimumExpectedWaveLaneCount: 0
// CHECK:       MaximumExpectedWaveLaneCount: 0
// CHECK:       ShaderFlags: 0 (None)
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) SignatureElementTable[5] = {
// CHECK:     <0:SignatureElement> = {
// CHECK:       SemanticName: "SV_Position"
// CHECK:       SemanticIndices: <0:array[1]> = { 0 }
// CHECK:       SemanticKind: Position
// CHECK:       ComponentType: F32
// CHECK:       InterpolationMode: LinearNoperspective
// CHECK:       StartRow: 0
// CHECK:       ColsAndStream: 3
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:     <1:SignatureElement> = {
// CHECK:       SemanticName: "TEXCOORD"
// CHECK:       SemanticIndices: <0:array[1]> = { 0 }
// CHECK:       SemanticKind: Arbitrary
// CHECK:       ComponentType: F32
// CHECK:       InterpolationMode: Linear
// CHECK:       StartRow: 1
// CHECK:       ColsAndStream: 1
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:     <2:SignatureElement> = {
// CHECK:       SemanticName: "NORMAL"
// CHECK:       SemanticIndices: <0:array[1]> = { 0 }
// CHECK:       SemanticKind: Arbitrary
// CHECK:       ComponentType: F32
// CHECK:       InterpolationMode: Linear
// CHECK:       StartRow: 2
// CHECK:       ColsAndStream: 2
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:     <3:SignatureElement> = {
// CHECK:       SemanticName: "SV_TessFactor"
// CHECK:       SemanticIndices: <2:array[3]> = { 0, 1, 2 }
// CHECK:       SemanticKind: TessFactor
// CHECK:       ComponentType: F32
// CHECK:       InterpolationMode: Undefined
// CHECK:       StartRow: 0
// CHECK:       ColsAndStream: 12
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:     <4:SignatureElement> = {
// CHECK:       SemanticName: "SV_InsideTessFactor"
// CHECK:       SemanticIndices: <0:array[1]> = { 0 }
// CHECK:       SemanticKind: InsideTessFactor
// CHECK:       ComponentType: F32
// CHECK:       InterpolationMode: Undefined
// CHECK:       StartRow: 3
// CHECK:       ColsAndStream: 0
// CHECK:       UsageAndDynIndexMasks: 0
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) HSInfoTable[2] = {
// CHECK:     <0:HSInfo> = {
// CHECK:       SigInputElements: <2:RecordArrayRef<SignatureElement>[3]>  = {
// CHECK:         [0]: <0:SignatureElement>
// CHECK:         [1]: <1:SignatureElement>
// CHECK:         [2]: <2:SignatureElement>
// CHECK:       }
// CHECK:       SigOutputElements: <RecordArrayRef<SignatureElement>[0]> = {}
// CHECK:       SigPatchConstOutputElements: <7:RecordArrayRef<SignatureElement>[2]>  = {
// CHECK:         [0]: <3:SignatureElement>
// CHECK:         [1]: <4:SignatureElement>
// CHECK:       }
// CHECK:       ViewIDOutputMask: <0:bytes[0]>
// CHECK:       ViewIDPatchConstOutputMask: <0:bytes[0]>
// CHECK:       InputToOutputMasks: <0:bytes[0]>
// CHECK:       InputToPatchConstOutputMasks: <0:bytes[0]>
// CHECK:       InputControlPointCount: 3
// CHECK:       OutputControlPointCount: 3
// CHECK:       TessellatorDomain: 2
// CHECK:       TessellatorOutputPrimitive: 3
// CHECK:     }
// CHECK:     <1:HSInfo> = {
// CHECK:       SigInputElements: <2:RecordArrayRef<SignatureElement>[3]>  = {
// CHECK:         [0]: <0:SignatureElement>
// CHECK:         [1]: <1:SignatureElement>
// CHECK:         [2]: <2:SignatureElement>
// CHECK:       }
// CHECK:       SigOutputElements: <RecordArrayRef<SignatureElement>[0]> = {}
// CHECK:       SigPatchConstOutputElements: <7:RecordArrayRef<SignatureElement>[2]>  = {
// CHECK:         [0]: <3:SignatureElement>
// CHECK:         [1]: <4:SignatureElement>
// CHECK:       }
// CHECK:       ViewIDOutputMask: <0:bytes[0]>
// CHECK:       ViewIDPatchConstOutputMask: <0:bytes[0]>
// CHECK:       InputToOutputMasks: <0:bytes[0]>
// CHECK:       InputToPatchConstOutputMasks: <0:bytes[0]>
// CHECK:       InputControlPointCount: 3
// CHECK:       OutputControlPointCount: 3
// CHECK:       TessellatorDomain: 2
// CHECK:       TessellatorOutputPrimitive: 4
// CHECK:     }
// CHECK:   }

// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     FunctionCount: 5
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?HSMain1{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK-NOT:     D3D12_FUNCTION_DESC: Name: \01?HSMain2{{[@$?.A-Za-z0-9_]+}}
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?HSMain3{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?HSPerPatchFunc1{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK-NOT:     D3D12_FUNCTION_DESC: Name: \01?HSPerPatchFunc2{{[@$?.A-Za-z0-9_]+}}
// CHECK:     D3D12_FUNCTION_DESC: Name: HSMain1
// CHECK:       Shader Version: Hull 6.3
// CHECK:       BoundResources: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK-NOT:     D3D12_FUNCTION_DESC: Name: HSMain2
// CHECK:     D3D12_FUNCTION_DESC: Name: HSMain3
// CHECK:       Shader Version: Hull 6.3
// CHECK:       BoundResources: 0
