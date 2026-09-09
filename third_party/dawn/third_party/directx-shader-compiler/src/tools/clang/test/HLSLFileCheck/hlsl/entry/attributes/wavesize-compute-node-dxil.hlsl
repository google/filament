// RUN: %dxc -T cs_6_6 -DNODE %s | FileCheck %s -check-prefixes=META,SM66
// RUN: %dxc -T cs_6_8 -DNODE %s | FileCheck %s -check-prefixes=META,SM68
// RUN: %dxc -T lib_6_6 %s | FileCheck %s -check-prefixes=META,SM66
// RUN: %dxc -T lib_6_8 -DNODE %s | FileCheck %s -check-prefixes=META,SM68,METANODE,METANODESM68

// RUN: %dxc -T cs_6_8 -DNODE -DRANGE=,64 %s | FileCheck %s -check-prefixes=META,SM68RANGE -DPREF=0
// RUN: %dxc -T lib_6_8 -DNODE -DRANGE=,64 %s | FileCheck %s -check-prefixes=META,SM68RANGE,METANODE,METANODESM68 -DPREF=0

// RUN: %dxc -T cs_6_8 -DNODE -DRANGE=,64,32 %s | FileCheck %s -check-prefixes=META,SM68RANGE -DPREF=32
// RUN: %dxc -T lib_6_8 -DNODE -DRANGE=,64,32 %s | FileCheck %s -check-prefixes=META,SM68RANGE,METANODE,METANODESM68 -DPREF=32

// META: @main, !"main", null, null, [[PROPS:![0-9]+]]}
// SM66: [[PROPS]] = !{
// SM66-SAME: i32 11, [[WS:![0-9]+]]
// SM68: [[PROPS]] = !{
// SM68-SAME: i32 23, [[WS:![0-9]+]]
// SM68RANGE: [[PROPS]] = !{
// SM68RANGE-SAME: i32 23, [[WS:![0-9]+]]
// SM66: [[WS]] = !{i32 16}
// SM68: [[WS]] = !{i32 16, i32 0, i32 0}
// SM68RANGE: [[WS]] = !{i32 16, i32 64, i32 [[PREF]]}

// METANODE: @node, !"node", null, null, [[PROPS:![0-9]+]]}
// METANODESM66: [[PROPS]] = !{
// METANODESM66-SAME: i32 11, [[WS]]
// METANODESM68: [[PROPS]] = !{
// METANODESM68-SAME: i32 23, [[WS]]


#ifndef RANGE
#define RANGE
#endif

[shader("compute")]
[wavesize(16 RANGE)]
[numthreads(1,1,8)]
void main() {
}

#ifdef NODE
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,8)]
[NodeDispatchGrid(1,1,1)]
[WaveSize(16 RANGE)]
void node() { }
#endif