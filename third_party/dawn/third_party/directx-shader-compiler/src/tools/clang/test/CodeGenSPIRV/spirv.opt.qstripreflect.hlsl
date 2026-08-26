// RUN: not %dxc -T ps_6_0 -E main -spirv -Qstrip_reflect -fcgl  %s -spirv 2>&1 | FileCheck %s

void main() {}

// CHECK: -Qstrip_reflect is not supported with -spirv
