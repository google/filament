// RUN: %dxc -T ms_6_5 -E main -fcgl  %s -spirv -fvk-invert-y | FileCheck %s -check-prefix=CHECK -check-prefix=INVERT
// RUN: %dxc -T ms_6_5 -E main -fcgl  %s -spirv | FileCheck %s -check-prefix=CHECK -check-prefix=NINVERT

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define NUM_THREADS 128
#define MAX_VERT 256
#define MAX_PRIM 256

// CHECK: [[v:%[0-9]+]] = OpConstantComposite %v4float %float_4 %float_5 %float_6 %float_7

[outputtopology("point")]
[numthreads(NUM_THREADS, 1, 1)]
void main(out vertices MeshPerVertex verts[MAX_VERT],
          out indices uint primitiveInd[MAX_PRIM],
          in uint tid : SV_DispatchThreadID)
{

    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);

// CHECK: [[glPosition:%[0-9]+]] = OpAccessChain %_ptr_Output_v4float %gl_Position %47
// NINVERT: OpStore [[glPosition]] [[v]]

// INVERT: [[vy:%[0-9]+]] = OpCompositeExtract %float [[v]] 1
// INVERT: [[nvy:%[0-9]+]] = OpFNegate %float [[vy]]
// INVERT: [[nv:%[0-9]+]] = OpCompositeInsert %v4float [[nvy]] [[v]] 1
// INVERT: OpStore [[glPosition]] [[nv]]
    verts[tid].position = float4(4.0,5.0,6.0,7.0);

    primitiveInd[6] = 2;
}

