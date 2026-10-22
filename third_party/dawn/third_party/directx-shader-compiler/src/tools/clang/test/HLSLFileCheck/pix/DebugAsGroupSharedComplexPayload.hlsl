// RUN: %dxc -enable-16bit-types -Od -Emain -Tas_6_6 %s | %opt -S -hlsl-dxil-PIX-add-tid-to-as-payload,dispatchArgY=3,dispatchArgZ=7 | %FileCheck %s

// Check that the payload was piece-wise copied into a local copy from group-shared:
// There are 28 elements:

//               CHECK: [[LOAD0:%.*]] = load [[TYPE0:.*]], [[TYPE0]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE0]]            [[LOAD0]]
//               CHECK: [[LOAD1:%.*]] = load [[TYPE1:.*]], [[TYPE1]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE1]]            [[LOAD1]]
//               CHECK: [[LOAD2:%.*]] = load [[TYPE2:.*]], [[TYPE2]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE2]]            [[LOAD2]]
//               CHECK: [[LOAD3:%.*]] = load [[TYPE3:.*]], [[TYPE3]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE3]]            [[LOAD3]]
//               CHECK: [[LOAD4:%.*]] = load [[TYPE4:.*]], [[TYPE4]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE4]]            [[LOAD4]]
//               CHECK: [[LOAD5:%.*]] = load [[TYPE5:.*]], [[TYPE5]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE5]]            [[LOAD5]]
//               CHECK: [[LOAD6:%.*]] = load [[TYPE6:.*]], [[TYPE6]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE6]]            [[LOAD6]]
//               CHECK: [[LOAD7:%.*]] = load [[TYPE7:.*]], [[TYPE7]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE7]]            [[LOAD7]]
//               CHECK: [[LOAD8:%.*]] = load [[TYPE8:.*]], [[TYPE8]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE8]]            [[LOAD8]]
//               CHECK: [[LOAD9:%.*]] = load [[TYPE9:.*]], [[TYPE9]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE9]]            [[LOAD9]]

//               CHECK: [[LOAD10:%.*]] = load [[TYPE10:.*]], [[TYPE10]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE10]]            [[LOAD10]]
//               CHECK: [[LOAD11:%.*]] = load [[TYPE11:.*]], [[TYPE11]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE11]]            [[LOAD11]]
//               CHECK: [[LOAD12:%.*]] = load [[TYPE12:.*]], [[TYPE12]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE12]]            [[LOAD12]]
//               CHECK: [[LOAD13:%.*]] = load [[TYPE13:.*]], [[TYPE13]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE13]]            [[LOAD13]]
//               CHECK: [[LOAD14:%.*]] = load [[TYPE14:.*]], [[TYPE14]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE14]]            [[LOAD14]]
//               CHECK: [[LOAD15:%.*]] = load [[TYPE15:.*]], [[TYPE15]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE15]]            [[LOAD15]]
//               CHECK: [[LOAD16:%.*]] = load [[TYPE16:.*]], [[TYPE16]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE16]]            [[LOAD16]]
//               CHECK: [[LOAD17:%.*]] = load [[TYPE17:.*]], [[TYPE17]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE17]]            [[LOAD17]]
//               CHECK: [[LOAD18:%.*]] = load [[TYPE18:.*]], [[TYPE18]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE18]]            [[LOAD18]]
//               CHECK: [[LOAD19:%.*]] = load [[TYPE19:.*]], [[TYPE19]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE19]]            [[LOAD19]]

//               CHECK: [[LOAD20:%.*]] = load [[TYPE20:.*]], [[TYPE20]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE20]]            [[LOAD20]]
//               CHECK: [[LOAD21:%.*]] = load [[TYPE21:.*]], [[TYPE21]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE21]]            [[LOAD21]]
//               CHECK: [[LOAD22:%.*]] = load [[TYPE22:.*]], [[TYPE22]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE22]]            [[LOAD22]]
//               CHECK: [[LOAD23:%.*]] = load [[TYPE23:.*]], [[TYPE23]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE23]]            [[LOAD23]]
//               CHECK: [[LOAD24:%.*]] = load [[TYPE24:.*]], [[TYPE24]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE24]]            [[LOAD24]]
//               CHECK: [[LOAD25:%.*]] = load [[TYPE25:.*]], [[TYPE25]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE25]]            [[LOAD25]]
//               CHECK: [[LOAD26:%.*]] = load [[TYPE26:.*]], [[TYPE26]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE26]]            [[LOAD26]]
//               CHECK: [[LOAD27:%.*]] = load [[TYPE27:.*]], [[TYPE27]] addrspace(3)* getelementptr inbounds
// CHECK:store volatile [[TYPE27]]            [[LOAD27]]

// And no more:
// CHECK-NOT: [[LOAD28:%.*]] = load [[TYPE28:.*]], [[TYPE28]] addrspace(3)* getelementptr inbounds

struct Contained {
  uint j;
  float af[3];
};

struct Bigger {
  half h;
  Contained a[2];
};

struct MyPayload {
  uint i;
  Bigger big[3];
};

groupshared MyPayload payload;

[numthreads(1, 1, 1)] void main(uint gid
                                : SV_GroupID) {
  DispatchMesh(1, 1, 1, payload);
}
