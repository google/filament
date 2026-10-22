// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Test common input parameters like SV_DispatchThreadID works.

// CHECK: define void @node01()
// CHECK:  %[[ftid:.+]] = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)  ; FlattenedThreadIdInGroup()

// CHECK:  %[[Hdl:.+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 250, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK:  %[[annotHdl:.+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle %[[Hdl]], %dx.types.NodeRecordInfo { i32 69, i32 16 })  ; AnnotateNodeRecordHandle(noderecord,props)

// CHECK:  %[[tid_group_z:.+]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 2)  ; ThreadIdInGroup(component)
// CHECK:  %[[tid_group_y:.+]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 1)  ; ThreadIdInGroup(component)
// CHECK:  %[[tid_group_x:.+]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)  ; ThreadIdInGroup(component)



// CHECK:  %[[node_ptr:.+]] = call %struct.RECORD.0 addrspace(6)* @dx.op.getNodeRecordPtr.struct.RECORD.0(i32 239, %dx.types.NodeRecordHandle %[[annotHdl]], i32 0)  ; GetNodeRecordPtr(recordhandle,arrayIndex)

// CHECK:  %[[ptr:.+]] = getelementptr %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 0
// CHECK:  store i32 0, i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 1, i32 0
// CHECK:  store i32 %[[tid_group_x]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 1, i32 1
// CHECK:  store i32 %[[tid_group_y]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 1, i32 2
// CHECK:  store i32 %[[tid_group_z]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  ret void

struct RECORD
{
  uint gidx;
  uint3 gtid;
};

[Shader("node")]
[numthreads(4,4,4)]
[NodeLaunch("coalescing")]
void node01(RWGroupNodeInputRecords<RECORD> input,

 uint GIdx : SV_GroupIndex,
 uint3 GTID : SV_GroupThreadID )
{
  if (GIdx != 0)
    return;
  input.Get().gidx = GIdx;
  input.Get().gtid = GTID;
}
