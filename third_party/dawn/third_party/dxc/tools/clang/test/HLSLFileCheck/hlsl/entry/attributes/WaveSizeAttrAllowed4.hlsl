// RUN: %dxc -T cs_6_6 -E csmain1 -ast-dump %s | FileCheck %s -check-prefixes=CHECK-MAIN1
// RUN: %dxc -T cs_6_6 -E csmain2 -ast-dump %s | FileCheck %s -check-prefixes=CHECK-MAIN2

// another regression test that we should be sure compiles correctly

// CHECK-MAIN1: -FunctionDecl
// CHECK-MAIN1-SAME: csmain1
// CHECK-MAIN1: -HLSLWaveSizeAttr
// CHECK-MAIN1-SAME: 32
// CHECK-MAIN1: -FunctionDecl
// CHECK-MAIN1-SAME: csmain2

// CHECK-MAIN2: -FunctionDecl
// CHECK-MAIN2: -FunctionDecl
// CHECK-MAIN2-SAME: csmain2
// CHECK-MAIN2: -HLSLWaveSizeAttr
// CHECK-MAIN2-SAME: 32


[WaveSize(32)]
[numthreads(32, 1, 1)]
void csmain1(){ }

[WaveSize(32)]
[numthreads(32, 1, 1)]
void csmain2(){ }
