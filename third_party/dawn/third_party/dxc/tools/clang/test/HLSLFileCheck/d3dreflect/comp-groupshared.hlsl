// RUN: %dxc -T lib_6_5 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

// CHECK:DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:  StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:  IndexTable (size = {{[0-9]+}} bytes)
// CHECK:  RawBytes (size = {{[0-9]+}} bytes)
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) ResourceTable[2] = {
// CHECK:    <0:RuntimeDataResourceInfo> = {
// CHECK:      Class: SRV
// CHECK:      Kind: StructuredBuffer
// CHECK:      ID: 0
// CHECK:      Space: 0
// CHECK:      LowerBound: 1
// CHECK:      UpperBound: 1
// CHECK:      Name: "inputs"
// CHECK:      Flags: 0 (None)
// CHECK:    }
// CHECK:    <1:RuntimeDataResourceInfo> = {
// CHECK:      Class: UAV
// CHECK:      Kind: TypedBuffer
// CHECK:      ID: 0
// CHECK:      Space: 0
// CHECK:      LowerBound: 1
// CHECK:      UpperBound: 1
// CHECK:      Name: "g_Intensities"
// CHECK:      Flags: 0 (None)
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[1] = {
// CHECK:    <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:      Name: "main"
// CHECK:      UnmangledName: "main"
// CHECK:      Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:        [0]: <0:RuntimeDataResourceInfo> = {
// CHECK:          Class: SRV
// CHECK:          Kind: StructuredBuffer
// CHECK:          ID: 0
// CHECK:          Space: 0
// CHECK:          LowerBound: 1
// CHECK:          UpperBound: 1
// CHECK:          Name: "inputs"
// CHECK:          Flags: 0 (None)
// CHECK:        }
// CHECK:        [1]: <1:RuntimeDataResourceInfo> = {
// CHECK:          Class: UAV
// CHECK:          Kind: TypedBuffer
// CHECK:          ID: 0
// CHECK:          Space: 0
// CHECK:          LowerBound: 1
// CHECK:          UpperBound: 1
// CHECK:          Name: "g_Intensities"
// CHECK:          Flags: 0 (None)
// CHECK:        }
// CHECK:      }
// CHECK:      FunctionDependencies: <string[0]> = {}
// CHECK:      ShaderKind: Compute
// CHECK:      PayloadSizeInBytes: 0
// CHECK:      AttributeSizeInBytes: 0
// CHECK:      FeatureInfo1: 0
// CHECK:      FeatureInfo2: (Opt_RequiresGroup)
// CHECK:      ShaderStageFlag: (Compute)
// CHECK:      MinShaderTarget: 0x50060
// CHECK:      MinimumExpectedWaveLaneCount: 0
// CHECK:      MaximumExpectedWaveLaneCount: 0
// CHECK:      ShaderFlags: 0 (None)
// CHECK:      CS: <0:CSInfo> = {
// CHECK:        NumThreads: <3:array[3]> = { 64, 2, 2 }
// CHECK:        GroupSharedBytesUsed: 16
// CHECK:      }
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) CSInfoTable[1] = {
// CHECK:    <0:CSInfo> = {
// CHECK:      NumThreads: <3:array[3]> = { 64, 2, 2 }
// CHECK:      GroupSharedBytesUsed: 16
// CHECK:    }
// CHECK:  }
// CHECK:ID3D12LibraryReflection:
// CHECK:  D3D12_LIBRARY_DESC:
// CHECK:    Creator: <nullptr>
// CHECK:    Flags: 0
// CHECK:    FunctionCount: 1
// CHECK:  ID3D12FunctionReflection:
// CHECK:    D3D12_FUNCTION_DESC: Name: main
// CHECK:      Shader Version: Compute 6.5
// CHECK:      Creator: <nullptr>
// CHECK:      Flags: 0
// CHECK:      ConstantBuffers: 1
// CHECK:      BoundResources: 2
// CHECK:      FunctionParameterCount: 0
// CHECK:      HasReturn: FALSE
// CHECK:    Bound Resources:
// CHECK:      D3D12_SHADER_INPUT_BIND_DESC: Name: inputs
// CHECK:        Type: D3D_SIT_STRUCTURED
// CHECK:        uID: 0
// CHECK:        BindCount: 1
// CHECK:        BindPoint: 1
// CHECK:        Space: 0
// CHECK:        ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:        Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:        NumSamples (or stride): 16
// CHECK:        uFlags: 0
// CHECK:      D3D12_SHADER_INPUT_BIND_DESC: Name: g_Intensities
// CHECK:        Type: D3D_SIT_UAV_RWTYPED
// CHECK:        uID: 0
// CHECK:        BindCount: 1
// CHECK:        BindPoint: 1
// CHECK:        Space: 0
// CHECK:        ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:        Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:        NumSamples (or stride): 4294967295
// CHECK:        uFlags: 0

struct Foo {
    int a[2];
    int d[2];
};

StructuredBuffer<Foo> inputs : register(t1);
RWBuffer< int > g_Intensities : register(u1);

groupshared Foo sharedData;

[shader("compute")]
[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
  if (GI==0)
	sharedData = inputs[GI];
	int rtn;
	InterlockedAdd(sharedData.d[0], g_Intensities[GI], rtn);
	g_Intensities[GI] = rtn + sharedData.d[0];
}
