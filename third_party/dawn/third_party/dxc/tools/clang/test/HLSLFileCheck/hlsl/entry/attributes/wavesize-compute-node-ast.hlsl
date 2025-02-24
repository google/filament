// RUN: %dxc -T cs_6_6 -DNODE -ast-dump %s | FileCheck %s -check-prefixes=AST,AST1,ASTNODE,ASTNODE1
// RUN: %dxc -T cs_6_8 -DNODE -ast-dump %s | FileCheck %s -check-prefixes=AST,AST1,ASTNODE,ASTNODE1
// RUN: %dxc -T lib_6_6 -ast-dump %s | FileCheck %s -check-prefixes=AST,AST1
// RUN: %dxc -T lib_6_8 -DNODE -ast-dump %s | FileCheck %s -check-prefixes=AST,AST1,ASTNODE,ASTNODE1

// RUN: %dxc -T cs_6_8 -DNODE -DRANGE=,64 -ast-dump %s | FileCheck %s -check-prefixes=AST,AST2,ASTNODE,ASTNODE2 -DPREF=0
// RUN: %dxc -T lib_6_8 -DNODE -DRANGE=,64 -ast-dump %s | FileCheck %s -check-prefixes=AST,AST2,ASTNODE,ASTNODE2 -DPREF=0

// RUN: %dxc -T cs_6_8 -DNODE -DRANGE=,64,32 -ast-dump %s | FileCheck %s -check-prefixes=AST,AST2,ASTNODE,ASTNODE2 -DPREF=32
// RUN: %dxc -T lib_6_8 -DNODE -DRANGE=,64,32 -ast-dump %s | FileCheck %s -check-prefixes=AST,AST2,ASTNODE,ASTNODE2 -DPREF=32


// Notes on RUN variations:
//  - Tests cs and lib with SM 6.6 and SM 6.8, with limitations for SM 6.6:
//    - Node shader excluded from lib_6_6, or node entry is not compat.
//      - another way would be to specify exports, but that complicates things,
//        since it will cause second callable main export
//    - Means RDAT checks may assume SM 6.8
//    - Limited to legacy form (1)
//  - -DRANGE used with range form to specify Max and optionally Preferred
//  - -DPREF=N used for FileCheck to check matching preference value, 0 or 32

// AST-LABEL: main 'void ()'
// AST1: -HLSLWaveSizeAttr
// AST1-SAME: 16 0 0
// AST2: -HLSLWaveSizeAttr
// AST2-SAME: 16 64 [[PREF]]

// ASTNODE-LABEL: node 'void ()'
// ASTNODE1: -HLSLWaveSizeAttr
// ASTNODE1-SAME: 16 0 0
// ASTNODE2: -HLSLWaveSizeAttr
// ASTNODE2-SAME: 16 64 [[PREF]]

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