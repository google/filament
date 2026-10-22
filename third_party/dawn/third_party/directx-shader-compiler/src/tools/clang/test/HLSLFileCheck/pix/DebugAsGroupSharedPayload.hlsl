// RUN: %dxc -Od -Emain -Tas_6_6 %s | %opt -S -hlsl-dxil-PIX-add-tid-to-as-payload,dispatchArgY=3,dispatchArgZ=7 | %FileCheck %s

// Check that the payload was piece-wise copied into a local copy
// CHECK: [[LOADGEP:%.*]] = getelementptr %struct.MyPayload
// CHECK: [[LOAD:%.*]] = load i32, i32* [[LOADGEP]]
// CHECK: store volatile i32 [[LOAD]]

struct MyPayload
{
    uint i;
};

groupshared MyPayload payload;

[numthreads(1, 1, 1)]
void main(uint gid : SV_GroupID)
{
  MyPayload copy;
  copy = payload;
  DispatchMesh(1, 1, 1, copy);
}
