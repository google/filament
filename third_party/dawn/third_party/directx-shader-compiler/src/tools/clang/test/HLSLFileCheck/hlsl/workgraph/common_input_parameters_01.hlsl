// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Test common input parameters like SV_DispatchThreadID works.

// CHECK: define void @node01()
// CHECK:  %[[ftid:.+]] = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)  ; FlattenedThreadIdInGroup()

// CHECK:  %[[Hdl:.+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 250, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK:  %[[annotHdl:.+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle %[[Hdl]], %dx.types.NodeRecordInfo { i32 101, i32 52 })  ; AnnotateNodeRecordHandle(noderecord,props)

// CHECK:  %[[tid_group_z:.+]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 2)  ; ThreadIdInGroup(component)
// CHECK:  %[[tid_group_y:.+]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 1)  ; ThreadIdInGroup(component)
// CHECK:  %[[tid_group_x:.+]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)  ; ThreadIdInGroup(component)
// CHECK:  %[[gid_z:.+]] = call i32 @dx.op.groupId.i32(i32 94, i32 2)  ; GroupId(component)
// CHECK:  %[[gid_y:.+]] = call i32 @dx.op.groupId.i32(i32 94, i32 1)  ; GroupId(component)
// CHECK:  %[[gid_x:.+]] = call i32 @dx.op.groupId.i32(i32 94, i32 0)  ; GroupId(component)
// CHECK:  %[[tid_z:.+]] = call i32 @dx.op.threadId.i32(i32 93, i32 2)  ; ThreadId(component)
// CHECK:  %[[tid_y:.+]] = call i32 @dx.op.threadId.i32(i32 93, i32 1)  ; ThreadId(component)
// CHECK:  %[[tid_x:.+]] = call i32 @dx.op.threadId.i32(i32 93, i32 0)  ; ThreadId(component)


// CHECK:  %[[node_ptr:.+]] = call %struct.RECORD.0 addrspace(6)* @dx.op.getNodeRecordPtr.struct.RECORD.0(i32 239, %dx.types.NodeRecordHandle %[[annotHdl]], i32 0)  ; GetNodeRecordPtr(recordhandle,arrayIndex)
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 0, i32 0
// CHECK:  store i32 %[[tid_x]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 0, i32 1
// CHECK:  store i32 %[[tid_y]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 0, i32 2
// CHECK:  store i32 %[[tid_z]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 1, i32 0
// CHECK:  store i32 %[[gid_x]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 1, i32 1
// CHECK:  store i32 %[[gid_y]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 1, i32 2
// CHECK:  store i32 %[[gid_z]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 2
// CHECK:  store i32 0, i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 3, i32 0
// CHECK:  store i32 %[[tid_group_x]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 3, i32 1
// CHECK:  store i32 %[[tid_group_y]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  %[[ptr:.+]] = getelementptr inbounds %struct.RECORD.0, %struct.RECORD.0 addrspace(6)* %[[node_ptr]], i32 0, i32 3, i32 2
// CHECK:  store i32 %[[tid_group_z]], i32 addrspace(6)* %[[ptr]], align 4
// CHECK:  ret void

struct RECORD
{
  uint3 dtid;
  uint3 gid;
  uint gidx;
  uint3 gtid;
  uint3 dg : SV_DispatchGrid;
};


struct INPUT
{
  uint3 dtid: SV_DispatchThreadID;
  uint3 gid: SV_GroupID;
  uint gidx: SV_GroupIndex;
  uint3 gtid: SV_GroupThreadID;
};

[Shader("node")]
[numthreads(4,4,4)]
[NodeMaxDispatchGrid(4,4,4)]
[NodeLaunch("broadcasting")]
void node01(RWDispatchNodeInputRecord<RECORD> input,
 INPUT r )
{
  if (r.gidx != 0)
    return;

  input.Get().dtid = r.dtid;
  input.Get().gid = r.gid;
  input.Get().gidx = r.gidx;
  input.Get().gtid = r.gtid;
}
