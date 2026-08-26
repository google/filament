// RUN: %dxc -enable-16bit-types -Od -Emain -Tas_6_6 %s | %opt -S -hlsl-dxil-PIX-add-tid-to-as-payload,dispatchArgY=3,dispatchArgZ=7 | %FileCheck %s

// Check that the payload was piece-wise copied into a local copy from group-shared:
// There are only 2 elements (the bitfield should take up 1 uint slot)

//               CHECK: [[LOAD0:%.*]] = load [[TYPE0:.*]], [[TYPE0]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE0]]            [[LOAD0]]
//               CHECK: [[LOAD1:%.*]] = load [[TYPE1:.*]], [[TYPE1]] addrspace(3)* getelementptr inbounds 
// CHECK:store volatile [[TYPE1]]            [[LOAD1]]

// And no more:
// CHECK-NOT: [[LOAD2:%.*]] = load {{.*}}, {{.*}} addrspace(3)* getelementptr inbounds

struct MyPayload {
  uint i;
  void Init() { i = 27; }
struct {
  int bf0 : 7;
  int bf1 : 11;
} bitfields;
};

groupshared MyPayload payload;

[numthreads(1, 1, 1)] void main(uint gid
                                : SV_GroupID) {
  DispatchMesh(1, 1, 1, payload);
}
