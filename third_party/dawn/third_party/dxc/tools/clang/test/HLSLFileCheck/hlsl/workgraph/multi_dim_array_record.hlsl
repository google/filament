// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -D USE_VECTOR %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -D USE_VECTOR -Od %s | FileCheck %s

// Ensure multi-dim array produced by lowering record struct with array of
// vector has GEP instructions merged to prevent invalid GEP with array type.

// CHECK: [[TID:%[0-9]+]] = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
// CHECK-DAG: [[NOH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 247, i32 0)
// CHECK-DAG: [[ANOH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 249, %dx.types.NodeHandle [[NOH]], %dx.types.NodeInfo { i32 6, i32 32768 })
// CHECK-DAG: [[IRH:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 250, i32 0)
// CHECK-DAG: [[AIRH:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle [[IRH]], %dx.types.NodeRecordInfo { i32 97, i32 32768 })
// CHECK-DAG: [[ORH:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 238, %dx.types.NodeHandle [[ANOH]], i32 1, i1 false)
// CHECK-DAG: [[AORH:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle [[ORH]], %dx.types.NodeRecordInfo { i32 70, i32 32768 })
// CHECK-DAG: [[IRP:%[0-9]+]] = call %[[RECORD:struct\.MyRecord[^ ]*]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 239, %dx.types.NodeRecordHandle [[AIRH]], i32 0)
// CHECK-DAG: getelementptr inbounds %[[RECORD]], %[[RECORD]] addrspace(6)* [[IRP]], i32 0, i32 0, i32 [[TID]], i32 0
// CHECK-DAG: [[ORP:%[0-9]+]] = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 239, %dx.types.NodeRecordHandle [[AORH]], i32 0)
// CHECK-DAG: getelementptr inbounds %[[RECORD]], %[[RECORD]] addrspace(6)* [[ORP]], i32 0, i32 0, i32 [[TID]], i32 0

#define THREADGROUP_SIZE 1024
struct MyRecord {
#ifdef USE_VECTOR
    uint4 data[THREADGROUP_SIZE];
    uint4 data2[THREADGROUP_SIZE];
#else
    uint data[THREADGROUP_SIZE][4];
    uint data2[THREADGROUP_SIZE][4];
#endif
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(THREADGROUP_SIZE, 1, 1)]
void node1(
    DispatchNodeInputRecord<MyRecord> inputData,
    [MaxRecords(1)][NodeID("node2")] NodeOutput<MyRecord> child,
    uint threadInGroup : SV_GroupIndex
)
{
    GroupNodeOutputRecords<MyRecord> outRecs = child.GetGroupNodeOutputRecords(1);
    outRecs[0].data[threadInGroup] = inputData.Get().data[threadInGroup];
    outRecs[0].data2[threadInGroup] = inputData.Get().data2[threadInGroup];
    outRecs.OutputComplete();
}
