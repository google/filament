// RUN: %dxc -auto-binding-space 13 -T lib_6_3 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -auto-binding-space 13 -T lib_6_6 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

// Make sure usage flag is set properly for cbuffers used in libraries

// CHECK-NOT: CBufUnused

// CHECK: DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:   StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:   IndexTable (size = {{[0-9]+}} bytes)
// CHECK:   RawBytes (size = {{[0-9]+}} bytes)
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) ResourceTable[3] = {
// CHECK:     <0:RuntimeDataResourceInfo> = {
// CHECK:       Class: CBuffer
// CHECK:       Kind: CBuffer
// CHECK:       ID: 0
// CHECK:       Space: 13
// CHECK:       LowerBound: 0
// CHECK:       UpperBound: 0
// CHECK:       Name: "CBuf0"
// CHECK:       Flags: 0 (None)
// CHECK:     }
// CHECK:     <1:RuntimeDataResourceInfo> = {
// CHECK:       Class: CBuffer
// CHECK:       Kind: CBuffer
// CHECK:       ID: 1
// CHECK:       Space: 13
// CHECK:       LowerBound: 1
// CHECK:       UpperBound: 1
// CHECK:       Name: "CBuf1"
// CHECK:       Flags: 0 (None)
// CHECK:     }
// CHECK:     <2:RuntimeDataResourceInfo> = {
// CHECK:       Class: CBuffer
// CHECK:       Kind: CBuffer
// CHECK:       ID: 2
// CHECK:       Space: 13
// CHECK:       LowerBound: 2
// CHECK:       UpperBound: 4294967295
// CHECK:       Name: "CBuf2"
// CHECK:       Flags: 0 (None)
// CHECK:     }
// CHECK:   }
// CHECK:   RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[2] = {
// CHECK:     <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:       Name: "\01?foo{{[@$?.A-Za-z0-9_]+}}"
// CHECK:       UnmangledName: "foo"
// CHECK:       Resources: <0:RecordArrayRef<RuntimeDataResourceInfo>[1]>  = {
// CHECK:         [0]: <1:RuntimeDataResourceInfo>
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
// CHECK:       Name: "main"
// CHECK:       UnmangledName: "main"
// CHECK:       Resources: <2:RecordArrayRef<RuntimeDataResourceInfo>[2]>  = {
// CHECK:         [0]: <0:RuntimeDataResourceInfo>
// CHECK:         [1]: <2:RuntimeDataResourceInfo>
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
// CHECK:       ShaderFlags: 0 (None)
// CHECK:     }
// CHECK:   }

// CHECK: D3D12_SHADER_BUFFER_DESC: Name: CBuf1
// CHECK: Num Variables: 1
// CHECK: D3D12_SHADER_VARIABLE_DESC: Name: CBuf1
// CHECK: uFlags: (D3D_SVF_USED)
// CHECK: CBuffer: CBuf1

// CHECK: D3D12_SHADER_BUFFER_DESC: Name: CBuf0
// CHECK: Num Variables: 2
// CHECK: D3D12_SHADER_VARIABLE_DESC: Name: i1
// CHECK: uFlags: 0
// CHECK: D3D12_SHADER_VARIABLE_DESC: Name: f1
// CHECK: uFlags: (D3D_SVF_USED)
// CHECK: CBuffer: CBuf0

// CHECK: D3D12_SHADER_BUFFER_DESC: Name: CBuf2
// CHECK: Num Variables: 1
// CHECK: D3D12_SHADER_VARIABLE_DESC: Name: CBuf2
// CHECK: uFlags: (D3D_SVF_USED)
// CHECK: CBuffer: CBuf2

cbuffer CBuf0 {
  int i1;
  float f1;
}

struct CBStruct {
  int i2;
  float f2;
};

ConstantBuffer<CBStruct> CBuf1;
ConstantBuffer<CBStruct> CBufUnused;
ConstantBuffer<CBStruct> CBuf2[];

float unused_func() {
  return CBufUnused.i2;
}

export float foo() {
  return CBuf1.i2;
}

[shader("vertex")]
float main(int idx : IDX) : OUT {
  return f1 * CBuf2[idx].f2;
}
