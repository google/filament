// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s

// CHECK: %[[pld:[^ ]+]] = call %struct.MeshPayload{{[^ ]*}} @dx.op.getMeshPayload.struct.MeshPayload{{.*}}(i32 170)
// CHECK: call void @dx.op.setMeshOutputCounts(i32 168, i32 32, i32 16)
// CHECK: call void @dx.op.emitIndices

// Verify bool translated from mem type
// CHECK: %[[ppld0:[^ ]+]] = getelementptr inbounds %struct.MeshPayload{{[^ ]*}}, %struct.MeshPayload{{[^ ]*}}* %[[pld]], i32 0, i32 2, i32 0
// CHECK: %[[pld0:[^ ]+]] = load i32, i32* %[[ppld0]], align 4
// CHECK: %[[ppld1:[^ ]+]] = getelementptr inbounds %struct.MeshPayload{{[^ ]*}}, %struct.MeshPayload{{[^ ]*}}* %[[pld]], i32 0, i32 2, i32 1
// CHECK: %[[pld1:[^ ]+]] = load i32, i32* %[[ppld1]], align 4
// CHECK: %[[ppld2:[^ ]+]] = getelementptr inbounds %struct.MeshPayload{{[^ ]*}}, %struct.MeshPayload{{[^ ]*}}* %[[pld]], i32 0, i32 2, i32 2
// CHECK: %[[pld2:[^ ]+]] = load i32, i32* %[[ppld2]], align 4
// CHECK: %[[ppld3:[^ ]+]] = getelementptr inbounds %struct.MeshPayload{{[^ ]*}}, %struct.MeshPayload{{[^ ]*}}* %[[pld]], i32 0, i32 2, i32 3
// CHECK: %[[pld3:[^ ]+]] = load i32, i32* %[[ppld3]], align 4
// Inner components reversed due to column_major
// CHECK: icmp ne i32 %[[pld0]], 0
// CHECK: icmp ne i32 %[[pld2]], 0
// CHECK: icmp ne i32 %[[pld1]], 0
// CHECK: icmp ne i32 %[[pld3]], 0

// CHECK: call void @dx.op.storePrimitiveOutput
// CHECK: call void @dx.op.storeVertexOutput

// CHECK: ret void

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
