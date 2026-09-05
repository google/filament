// RUN: %dxc -EMSMain -Tms_6_6 %s | %opt -S -hlsl-dxil-pix-meshshader-output-instrumentation,expand-payload=0,dispatchArgY=3,dispatchArgZ=7,UAVSize=8192 | %FileCheck %s

// Check that the instrumentation calculates the expected "flat" group ID:

// CHECK: [[GROUPIDX:%.*]] = call i32 @dx.op.groupId.i32(i32 94, i32 0)
// CHECK: [[GROUPIDY:%.*]] = call i32 @dx.op.groupId.i32(i32 94, i32 1)
// CHECK: [[GROUPIDZ:%.*]] = call i32 @dx.op.groupId.i32(i32 94, i32 2)

// The integer literals here come from the dispatchArg* arguments in the command line above
// CHECK: [[YTIMES7:%.*]] = mul i32 [[GROUPIDY]], 7
// CHECK: [[ZPLUSYTIMES7:%.*]] = add i32 [[GROUPIDZ]], [[YTIMES7]]
// CHECK: [[XTIMES21:%.*]] = mul i32 [[GROUPIDX]], 21
// CHECK: [[FLATID:%.*]] = add i32 [[XTIMES21]], [[ZPLUSYTIMES7]]

// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[SOMERESOURCEHANDLE:%.*]], i32 [[SOMEOFFSET:%.*]], i32 undef, i32 [[FLATID]],
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
