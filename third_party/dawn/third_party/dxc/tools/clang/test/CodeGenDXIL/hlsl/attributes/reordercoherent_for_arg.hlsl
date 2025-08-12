// RUN: %dxc -E main -T lib_6_9 %s | FileCheck %s
// REQUIRES: dxil-1-9

// CHECK: %[[uH:[^ ]+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?u@@3V?$RWBuffer@M@@A", align 4
// CHECK: %[[uLIBH:[^ ]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %[[uH]]) ; CreateHandleForLib(Resource)
// CHECK: %[[uANNOT:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[uLIBH]], %dx.types.ResourceProperties { i32 69642, i32 265 }) ; AnnotateHandle(res,props) resource: reordercoherent RWTypedBuffer<F32>
// CHECK: %{{[^ ]+}} = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %[[uANNOT]], i32 0, i32 undef) ; BufferLoad(srv,index,wot)

RWBuffer<float> OutBuf : register(u1);
reordercoherent RWBuffer<float> u : register(u2);

float read(RWBuffer<float> buf) {
  return buf[0];
}

[shader("raygeneration")]
void main() {
  OutBuf[0] = read(u);
}
