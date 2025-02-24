// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float1x1 %s | FileCheck %s -check-prefix=CHK_TEST1
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float1x2 %s | FileCheck %s -check-prefix=CHK_TEST2
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float2x1 %s | FileCheck %s -check-prefix=CHK_TEST3
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float2x2 %s | FileCheck %s -check-prefix=CHK_TEST4
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float2x3 %s | FileCheck %s -check-prefix=CHK_TEST5
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float3x2 %s | FileCheck %s -check-prefix=CHK_TEST6
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float3x3 %s | FileCheck %s -check-prefix=CHK_TEST7
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float3x4 %s | FileCheck %s -check-prefix=CHK_TEST8
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float4x3 %s | FileCheck %s -check-prefix=CHK_TEST9
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTY=float4x4 %s | FileCheck %s -check-prefix=CHK_TEST10

// Regression test for github bug #3225

RWByteAddressBuffer buffer;

void main(uint i : IN0, TY t : IN1)
{
  // CHK_TEST1: dx.op.rawBufferStore.f32

  // CHK_TEST2: dx.op.rawBufferStore.f32

  // CHK_TEST3: dx.op.rawBufferStore.f32

  // CHK_TEST4: dx.op.rawBufferStore.f32

  // CHK_TEST5: dx.op.rawBufferStore.f32
  // CHK_TEST5: add i32 %{{.*}}, 16
  // CHK_TEST5: dx.op.rawBufferStore.f32
  
  // CHK_TEST6: dx.op.rawBufferStore.f32
  // CHK_TEST6: add i32 %{{.*}}, 16
  // CHK_TEST6: dx.op.rawBufferStore.f32
  
  // CHK_TEST7: dx.op.rawBufferStore.f32
  // CHK_TEST7: add i32 %{{.*}}, 16
  // CHK_TEST7: dx.op.rawBufferStore.f32
  // CHK_TEST7: add i32 %{{.*}}, 32
  // CHK_TEST7: dx.op.rawBufferStore.f32
  
  // CHK_TEST8: dx.op.rawBufferStore.f32
  // CHK_TEST8: add i32 %{{.*}}, 16
  // CHK_TEST8: dx.op.rawBufferStore.f32
  // CHK_TEST8: add i32 %{{.*}}, 32
  // CHK_TEST8: dx.op.rawBufferStore.f32
  
  // CHK_TEST9: dx.op.rawBufferStore.f32
  // CHK_TEST9: add i32 %{{.*}}, 16
  // CHK_TEST9: dx.op.rawBufferStore.f32
  // CHK_TEST9: add i32 %{{.*}}, 32
  // CHK_TEST9: dx.op.rawBufferStore.f32
  
  
  // CHK_TEST10: dx.op.rawBufferStore.f32
  // CHK_TEST10: add i32 %{{.*}}, 16
  // CHK_TEST10: dx.op.rawBufferStore.f32
  // CHK_TEST10: add i32 %{{.*}}, 32
  // CHK_TEST10: dx.op.rawBufferStore.f32
  // CHK_TEST10: add i32 %{{.*}}, 48
  // CHK_TEST10: dx.op.rawBufferStore.f32  

	buffer.Store(i, t);
}
