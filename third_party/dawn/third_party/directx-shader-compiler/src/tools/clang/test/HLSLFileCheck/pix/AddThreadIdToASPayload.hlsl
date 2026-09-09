// RUN: %dxc -Emain -Tas_6_6 %s | %opt -S -hlsl-dxil-PIX-add-tid-to-as-payload,dispatchArgY=3,dispatchArgZ=7 | %FileCheck %s

// CHECK: [[AppsGroupIdX:%.*]] = call i32 @dx.op.groupId.i32(i32 94, i32 0)
// CHECK: [[GROUPIDX:%.*]] = call i32 @dx.op.groupId.i32(i32 94, i32 0)
// CHECK: [[GROUPIDY:%.*]] = call i32 @dx.op.groupId.i32(i32 94, i32 1)
// CHECK: [[GROUPIDZ:%.*]] = call i32 @dx.op.groupId.i32(i32 94, i32 2)

// The integer literals here come from the dispatchArg* arguments in the command line above
// CHECK: [[YTIMES7:%.*]] = mul i32 [[GROUPIDY]], 7
// CHECK: [[ZPLUSYTIMES7:%.*]] = add i32 [[GROUPIDZ]], [[YTIMES7]]
// CHECK: [[XTIMES21:%.*]] = mul i32 [[GROUPIDX]], 21
// CHECK: [[FLATGROUPNUM:%.*]] = add i32 [[XTIMES21]], [[ZPLUSYTIMES7]]
// This 105 is the thread counts for the shader multiplied together
// CHECK: [[FLATWITHSPACE:%.*]] = mul i32 [[FLATGROUPNUM]], 105
// CHECK: [[FLATTENEDTHREADIDINGROUP:%.*]] = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
// CHECK: [[FLATID:%.*]] = add i32 [[FLATWITHSPACE]], [[FLATTENEDTHREADIDINGROUP]]

// Check that this flat ID is stored into the expanded payload:
// CHECKL store i32 [[FLATID]], i32*

// Check that the Y and Z dispatch-mesh counts are emitted to the expanded payload:
// CHECK: store i32 3, i32*
// CHECK: store i32 4, i32*
struct MyPayload
{
    uint i;
};

[numthreads(3, 5, 7)]
void main(uint gid : SV_GroupID)
{
    MyPayload payload;
    payload.i = gid;
    DispatchMesh(2, 3, 4, payload);
}
    