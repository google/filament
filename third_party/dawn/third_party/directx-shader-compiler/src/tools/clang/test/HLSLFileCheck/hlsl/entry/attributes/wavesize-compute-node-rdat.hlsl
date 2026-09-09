// RUN: %dxc -T lib_6_8 -DNODE %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT1
// RUN: %dxc -T lib_6_8 -DNODE -DRANGE=,64 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT2
// RUN: %dxc -T lib_6_8 -DNODE -DRANGE=,64,32 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT2


// RDAT has no min/max wave count until SM 6.8
// RDAT-LABEL: <0:RuntimeDataFunctionInfo{{.}}> = {
// RDAT: Name: "main"
// RDAT: MinimumExpectedWaveLaneCount: 16
// RDAT1: MaximumExpectedWaveLaneCount: 16
// RDAT2: MaximumExpectedWaveLaneCount: 64

// RDAT-LABEL: <1:RuntimeDataFunctionInfo{{.}}> = {
// RDAT: Name: "node"
// RDAT: MinimumExpectedWaveLaneCount: 16
// RDAT1: MaximumExpectedWaveLaneCount: 16
// RDAT2: MaximumExpectedWaveLaneCount: 64

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