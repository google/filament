// RUN: %dxc -T lib_6_7 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

// CHECK:DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:  StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:  IndexTable (size = {{[0-9]+}} bytes)
// CHECK:  RawBytes (size = {{[0-9]+}} bytes)
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[1] = {
// CHECK:    <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:      Name: "main"
// CHECK:      UnmangledName: "main"
// CHECK:      Resources: <RecordArrayRef<RuntimeDataResourceInfo>[0]> = {}
// CHECK:      FunctionDependencies: <string[0]> = {}
// CHECK:      ShaderKind: Mesh
// CHECK:      PayloadSizeInBytes: 0
// CHECK:      AttributeSizeInBytes: 0
// CHECK:      FeatureInfo1: (ViewID)
// CHECK:      FeatureInfo2: (Opt_RequiresGroup)
// CHECK:      ShaderStageFlag: (Mesh)
// CHECK:      MinShaderTarget: 0xd0065
// CHECK:      MinimumExpectedWaveLaneCount: 0
// CHECK:      MaximumExpectedWaveLaneCount: 0
// CHECK:      ShaderFlags: (OutputPositionPresent | UsesViewID)
// CHECK:      MS: <0:MSInfo> = {
// CHECK:        SigOutputElements: <7:RecordArrayRef<SignatureElement>[2]>  = {
// CHECK:          [0]: <0:SignatureElement> = {
// CHECK:            SemanticName: "SV_Position"
// CHECK:            SemanticIndices: <0:array[1]> = { 0 }
// CHECK:            SemanticKind: Position
// CHECK:            ComponentType: F32
// CHECK:            InterpolationMode: LinearNoperspective
// CHECK:            StartRow: 0
// CHECK:            ColsAndStream: 3
// CHECK:            UsageAndDynIndexMasks: 0
// CHECK:          }
// CHECK:          [1]: <1:SignatureElement> = {
// CHECK:            SemanticName: "COLOR"
// CHECK:            SemanticIndices: <2:array[4]> = { 0, 1, 2, 3 }
// CHECK:            SemanticKind: Arbitrary
// CHECK:            ComponentType: F32
// CHECK:            InterpolationMode: Linear
// CHECK:            StartRow: 1
// CHECK:            ColsAndStream: 0
// CHECK:            UsageAndDynIndexMasks: 0
// CHECK:          }
// CHECK:        }
// CHECK:        SigPrimOutputElements: <10:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:          [0]: <2:SignatureElement> = {
// CHECK:            SemanticName: "NORMAL"
// CHECK:            SemanticIndices: <0:array[1]> = { 0 }
// CHECK:            SemanticKind: Arbitrary
// CHECK:            ComponentType: F32
// CHECK:            InterpolationMode: Constant
// CHECK:            StartRow: 0
// CHECK:            ColsAndStream: 0
// CHECK:            UsageAndDynIndexMasks: 0
// CHECK:          }
// CHECK:        }
// CHECK:        ViewIDOutputMask: <0:bytes[0]>
// CHECK:        ViewIDPrimOutputMask: <0:bytes[0]>
// CHECK:        NumThreads: <12:array[3]> = { 32, 1, 1 }
// CHECK:        GroupSharedBytesUsed: 64
// CHECK:        GroupSharedBytesDependentOnViewID: 0
// CHECK:        PayloadSizeInBytes: 36
// CHECK:        MaxOutputVertices: 32
// CHECK:        MaxOutputPrimitives: 16
// CHECK:        MeshOutputTopology: 2
// CHECK:      }
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) SignatureElementTable[3] = {
// CHECK:    <0:SignatureElement> = {
// CHECK:      SemanticName: "SV_Position"
// CHECK:      SemanticIndices: <0:array[1]> = { 0 }
// CHECK:      SemanticKind: Position
// CHECK:      ComponentType: F32
// CHECK:      InterpolationMode: LinearNoperspective
// CHECK:      StartRow: 0
// CHECK:      ColsAndStream: 3
// CHECK:      UsageAndDynIndexMasks: 0
// CHECK:    }
// CHECK:    <1:SignatureElement> = {
// CHECK:      SemanticName: "COLOR"
// CHECK:      SemanticIndices: <2:array[4]> = { 0, 1, 2, 3 }
// CHECK:      SemanticKind: Arbitrary
// CHECK:      ComponentType: F32
// CHECK:      InterpolationMode: Linear
// CHECK:      StartRow: 1
// CHECK:      ColsAndStream: 0
// CHECK:      UsageAndDynIndexMasks: 0
// CHECK:    }
// CHECK:    <2:SignatureElement> = {
// CHECK:      SemanticName: "NORMAL"
// CHECK:      SemanticIndices: <0:array[1]> = { 0 }
// CHECK:      SemanticKind: Arbitrary
// CHECK:      ComponentType: F32
// CHECK:      InterpolationMode: Constant
// CHECK:      StartRow: 0
// CHECK:      ColsAndStream: 0
// CHECK:      UsageAndDynIndexMasks: 0
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) MSInfoTable[1] = {
// CHECK:    <0:MSInfo> = {
// CHECK:      SigOutputElements: <7:RecordArrayRef<SignatureElement>[2]>  = {
// CHECK:        [0]: <0:SignatureElement> = {
// CHECK:          SemanticName: "SV_Position"
// CHECK:          SemanticIndices: <0:array[1]> = { 0 }
// CHECK:          SemanticKind: Position
// CHECK:          ComponentType: F32
// CHECK:          InterpolationMode: LinearNoperspective
// CHECK:          StartRow: 0
// CHECK:          ColsAndStream: 3
// CHECK:          UsageAndDynIndexMasks: 0
// CHECK:        }
// CHECK:        [1]: <1:SignatureElement> = {
// CHECK:          SemanticName: "COLOR"
// CHECK:          SemanticIndices: <2:array[4]> = { 0, 1, 2, 3 }
// CHECK:          SemanticKind: Arbitrary
// CHECK:          ComponentType: F32
// CHECK:          InterpolationMode: Linear
// CHECK:          StartRow: 1
// CHECK:          ColsAndStream: 0
// CHECK:          UsageAndDynIndexMasks: 0
// CHECK:        }
// CHECK:      }
// CHECK:      SigPrimOutputElements: <10:RecordArrayRef<SignatureElement>[1]>  = {
// CHECK:        [0]: <2:SignatureElement> = {
// CHECK:          SemanticName: "NORMAL"
// CHECK:          SemanticIndices: <0:array[1]> = { 0 }
// CHECK:          SemanticKind: Arbitrary
// CHECK:          ComponentType: F32
// CHECK:          InterpolationMode: Constant
// CHECK:          StartRow: 0
// CHECK:          ColsAndStream: 0
// CHECK:          UsageAndDynIndexMasks: 0
// CHECK:        }
// CHECK:      }
// CHECK:      ViewIDOutputMask: <0:bytes[0]>
// CHECK:      ViewIDPrimOutputMask: <0:bytes[0]>
// CHECK:      NumThreads: <12:array[3]> = { 32, 1, 1 }
// CHECK:      GroupSharedBytesUsed: 64
// CHECK:      GroupSharedBytesDependentOnViewID: 0
// CHECK:      PayloadSizeInBytes: 36
// CHECK:      MaxOutputVertices: 32
// CHECK:      MaxOutputPrimitives: 16
// CHECK:      MeshOutputTopology: 2
// CHECK:    }
// CHECK:  }
// CHECK:ID3D12LibraryReflection:
// CHECK:  D3D12_LIBRARY_DESC:
// CHECK:    Creator: <nullptr>
// CHECK:    Flags: 0
// CHECK:    FunctionCount: 1
// CHECK:  ID3D12FunctionReflection:
// CHECK:    D3D12_FUNCTION_DESC: Name: main
// CHECK:      Shader Version: Mesh 6.7
// CHECK:      Creator: <nullptr>
// CHECK:      Flags: 0
// CHECK:      ConstantBuffers: 0
// CHECK:      BoundResources: 0
// CHECK:      FunctionParameterCount: 0
// CHECK:      HasReturn: FALSE

#define MAX_VERT 32
#define MAX_PRIM 16
#define NUM_THREADS 32
struct MeshPerVertex {
    float4 position : SV_Position;
    float color[4] : COLOR;
};

struct MeshPerPrimitive {
    float normal : NORMAL;
};

struct MeshPayload {
    float normal;
    int4 data;
    bool2x2 mat;
};

groupshared float gsMem[MAX_PRIM];

[numthreads(NUM_THREADS, 1, 1)]
[shader("mesh")]
[outputtopology("triangle")]
void main(
            out indices uint3 primIndices[MAX_PRIM],
            out vertices MeshPerVertex verts[MAX_VERT],
            out primitives MeshPerPrimitive prims[MAX_PRIM],
            in payload MeshPayload mpl,
            in uint tig : SV_GroupIndex,
            in uint vid : SV_ViewID
         )
{
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);
    MeshPerVertex ov;
    if (vid % 2) {
        ov.position = float4(4.0,5.0,6.0,7.0);
        ov.color[0] = 4.0;
        ov.color[1] = 5.0;
        ov.color[2] = 6.0;
        ov.color[3] = 7.0;
    } else {
        ov.position = float4(14.0,15.0,16.0,17.0);
        ov.color[0] = 14.0;
        ov.color[1] = 15.0;
        ov.color[2] = 16.0;
        ov.color[3] = 17.0;
    }
    if (tig % 3) {
        primIndices[tig / 3] = uint3(tig, tig + 1, tig + 2);
        MeshPerPrimitive op;
        op.normal = dot(mpl.normal.xx, mul(mpl.data.xy, mpl.mat));
        gsMem[tig / 3] = op.normal;
        prims[tig / 3] = op;
    }
    verts[tig] = ov;
}
