// RUN: %dxc -T lib_6_7 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s


// CHECK:DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:  StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:  IndexTable (size = {{[0-9]+}} bytes)
// CHECK:  RawBytes (size = {{[0-9]+}} bytes)
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) ResourceTable[1] = {
// CHECK:    <0:RuntimeDataResourceInfo> = {
// CHECK:      Class: SRV
// CHECK:      Kind: TypedBuffer
// CHECK:      ID: 0
// CHECK:      Space: 0
// CHECK:      LowerBound: 0
// CHECK:      UpperBound: 0
// CHECK:      Name: "g_tex"
// CHECK:      Flags: 0 (None)
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[1] = {
// CHECK:    <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:      Name: "amplification"
// CHECK:      UnmangledName: "amplification"
// CHECK:      Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[1]>  = {
// CHECK:        [0]: <0:RuntimeDataResourceInfo> = {
// CHECK:          Class: SRV
// CHECK:          Kind: TypedBuffer
// CHECK:          ID: 0
// CHECK:          Space: 0
// CHECK:          LowerBound: 0
// CHECK:          UpperBound: 0
// CHECK:          Name: "g_tex"
// CHECK:          Flags: 0 (None)
// CHECK:        }
// CHECK:      }
// CHECK:      FunctionDependencies: <string[0]> = {}
// CHECK:      ShaderKind: Amplification
// CHECK:      PayloadSizeInBytes: 0
// CHECK:      AttributeSizeInBytes: 0
// CHECK:      FeatureInfo1: 0
// CHECK:      FeatureInfo2: (Opt_RequiresGroup)
// CHECK:      ShaderStageFlag: (Amplification)
// CHECK:      MinShaderTarget: 0xe0065
// CHECK:      MinimumExpectedWaveLaneCount: 0
// CHECK:      MaximumExpectedWaveLaneCount: 0
// CHECK:      ShaderFlags: 0 (None)
// CHECK:      AS: <0:ASInfo> = {
// CHECK:        NumThreads: <2:array[3]> = { 4, 1, 1 }
// CHECK:        GroupSharedBytesUsed: 32
// CHECK:        PayloadSizeInBytes: 16
// CHECK:      }
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) ASInfoTable[1] = {
// CHECK:    <0:ASInfo> = {
// CHECK:      NumThreads: <2:array[3]> = { 4, 1, 1 }
// CHECK:      GroupSharedBytesUsed: 32
// CHECK:      PayloadSizeInBytes: 16
// CHECK:    }
// CHECK:  }
// CHECK:ID3D12LibraryReflection:
// CHECK:  D3D12_LIBRARY_DESC:
// CHECK:    Flags: 0
// CHECK:    FunctionCount: 1
// CHECK:  ID3D12FunctionReflection:
// CHECK:    D3D12_FUNCTION_DESC: Name: amplification
// CHECK:      Shader Version: Amplification 6.7
// CHECK:      Flags: 0
// CHECK:      ConstantBuffers: 0
// CHECK:      BoundResources: 1
// CHECK:      FunctionParameterCount: 0
// CHECK:      HasReturn: FALSE
// CHECK:    Bound Resources:
// CHECK:      D3D12_SHADER_INPUT_BIND_DESC: Name: g_tex
// CHECK:        Type: D3D_SIT_TEXTURE
// CHECK:        uID: 0
// CHECK:        BindCount: 1
// CHECK:        BindPoint: 0
// CHECK:        Space: 0
// CHECK:        ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK:        Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:        NumSamples (or stride): 4294967295
// CHECK:        uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)

struct MeshPayload
{
  float4 data;
};

groupshared MeshPayload pld;
groupshared float4 gs_bias;
Buffer<float4> g_tex : register(t0);

[shader("amplification")]
[numthreads(4,1,1)]
void amplification(uint gtid : SV_GroupIndex)
{
  pld.data = g_tex[gtid]*gs_bias;
  gs_bias += 0.1;
  DispatchMesh(1,1,1,pld);
}
