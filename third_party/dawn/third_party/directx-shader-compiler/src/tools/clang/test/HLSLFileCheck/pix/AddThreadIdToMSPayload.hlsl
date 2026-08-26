// RUN: %dxc -EMSMain -Tms_6_6 %s | %opt -S -hlsl-dxil-pix-meshshader-output-instrumentation,expand-payload=1,UAVSize=8192 | %FileCheck %s

// CHECK: getelementptr %PIX_AS2MS_Expanded_Type

// Validate that instrumentation appeared before a store-output
// CHECK: call void @dx.op.bufferStore.i32
// CHECK: call void @dx.op.storeVertexOutput

struct PSInput
{
    float4 position : SV_POSITION;
};

struct MyPayload
{
    uint i;
};

[outputtopology("triangle")]
[numthreads(4, 1, 1)]
void MSMain(
    in payload MyPayload small,
    in uint tid : SV_GroupThreadID,
    out vertices PSInput verts[4],
    out indices uint3 triangles[2])
{
    SetMeshOutputCounts(4, 2);
    verts[tid].position = float4(small.i, 0, 0, 0);
    triangles[tid % 2] = uint3(0, tid + 1, tid + 2);
}
