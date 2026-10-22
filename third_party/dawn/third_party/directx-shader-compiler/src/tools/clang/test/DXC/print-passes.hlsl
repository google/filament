// RUN: %dxc -E main -T vs_6_0 %s -print-before-all 2>&1 | FileCheck -check-prefix=BEFORE1 --check-prefix=BEFORE2 %s
// RUN: %dxc -E main -T vs_6_0 %s -print-after-all 2>&1 | FileCheck -check-prefix=AFTER1 --check-prefix=AFTER2 %s
// RUN: %dxc -E main -T vs_6_0 %s -print-before-all -print-after-all 2>&1 | FileCheck -check-prefix=BEFORE1 -check-prefix=BEFORE2 -check-prefix=AFTER1 --check-prefix=AFTER2 %s
// RUN: %dxc -E main -T vs_6_0 %s -print-before verify 2>&1 | FileCheck -check-prefix=BEFORE1 %s
// RUN: %dxc -E main -T vs_6_0 %s -print-after hlsl-dxilemit 2>&1 | FileCheck -check-prefix=AFTER2 %s
// RUN: %dxc -E main -T vs_6_0 %s -print-before verify -print-before hlsl-dxilemit 2>&1 | FileCheck -check-prefix=BEFORE1 -check-prefix=BEFORE2 %s
// RUN: %dxc -E main -T vs_6_0 %s -print-after hlsl-dxilemit -print-after verify 2>&1 | FileCheck -check-prefix=AFTER1 -check-prefix=AFTER2 %s
// RUN: %dxc -E main -T vs_6_0 %s -print-after hlsl-dxilemit -print-before verify 2>&1 | FileCheck -check-prefix=BEFORE1 -check-prefix=AFTER2 %s

// BEFORE1: *** IR Dump Before Module Verifier (verify) ***
// BEFORE1: define void @main
// AFTER1: *** IR Dump After Module Verifier (verify) ***
// AFTER1: define void @main
// BEFORE2: *** IR Dump Before HLSL DXIL Metadata Emit (hlsl-dxilemit) ***
// BEFORE2: define void @main
// AFTER2: *** IR Dump After HLSL DXIL Metadata Emit (hlsl-dxilemit) ***
// AFTER2: define void @main

void main() {}
