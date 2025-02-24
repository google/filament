// RUN: not %dxc -T ps_6_6 -E main -fcgl  %s -spirv -enable-16bit-types  2>&1 | FileCheck %s

// CHECK: error: 16-bit texture types not yet supported with -spirv
Buffer<half> MyBuffer;

void main(): SV_Target { }
