// RUN: %dxc -E main -T lib_6_9 %s | FileCheck %s
// REQUIRES: dxil-1-9

// Make sure uav array can have reordercoherent.
// CHECK: !{{.*}} = !{i32 1, [12 x %"class.RWTexture2D<float>"]* bitcast ([12 x %dx.types.Handle]* @"\01?tex@@3PAV?$RWTexture2D@M@@A" to [12 x %"class.RWTexture2D<float>"]*), !"tex", i32 0, i32 2, i32 12, i32 2, i1 false, i1 false, i1 false, ![[TAGMD:.*]]}
// CHECK: ![[TAGMD]] = !{i32 0, i32 9, i32 4, i1 true}


RWBuffer<float> OutBuf: register(u1);
reordercoherent RWTexture2D<float> tex[12] : register(u2);

[shader("raygeneration")]
void main() {
  int2 c = DispatchRaysIndex().xy;
  OutBuf[0] = tex[0][c];
}
