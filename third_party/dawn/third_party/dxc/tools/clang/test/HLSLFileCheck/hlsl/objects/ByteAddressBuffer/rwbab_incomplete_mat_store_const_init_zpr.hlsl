// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST1=1 %s | FileCheck %s -check-prefix=CHK_TEST1
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST2=1 %s | FileCheck %s -check-prefix=CHK_TEST2
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST3=1 %s | FileCheck %s -check-prefix=CHK_TEST3
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST4=1 %s | FileCheck %s -check-prefix=CHK_TEST4
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST5=1 %s | FileCheck %s -check-prefix=CHK_TEST5
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST6=1 %s | FileCheck %s -check-prefix=CHK_TEST6
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST7=1 %s | FileCheck %s -check-prefix=CHK_TEST7
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST8=1 %s | FileCheck %s -check-prefix=CHK_TEST8
// RUN: %dxc -E main -T vs_6_5 -Zpr -DTEST9=1 %s | FileCheck %s -check-prefix=CHK_TEST9
// RUN: %dxc -E main -T vs_6_5 -Zpr %s | FileCheck %s -check-prefix=CHK_TEST10

// Regression test for github bug #3225

RWByteAddressBuffer buffer;

void main()
{
#ifdef TEST1
  // CHK_TEST1: dx.op.rawBufferStore.f32
  // CHK_TEST1: i32 0, i32 undef, float 1.000000e+00
  float1x1 t = {1};
#elif TEST2
  // CHK_TEST2: dx.op.rawBufferStore.f32
  // CHK_TEST2: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00
  float1x2 t = {1,2};
#elif TEST3  
  // CHK_TEST3: dx.op.rawBufferStore.f32
  // CHK_TEST3: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00
  float2x1 t = {1,2};
#elif TEST4  
  // CHK_TEST4: dx.op.rawBufferStore.f32
  // CHK_TEST4: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00
  float2x2 t = {1,2,3,4};
#elif TEST5  
  // CHK_TEST5: dx.op.rawBufferStore.f32
  // CHK_TEST5: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00
  // CHK_TEST5: dx.op.rawBufferStore.f32
  // CHK_TEST5: i32 16, i32 undef, float 5.000000e+00, float 6.000000e+00
  float2x3 t = {1,2,3,4,5,6};
#elif TEST6
  // CHK_TEST6: dx.op.rawBufferStore.f32
  // CHK_TEST6: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00
  // CHK_TEST6: dx.op.rawBufferStore.f32
  // CHK_TEST6: i32 16, i32 undef, float 5.000000e+00, float 6.000000e+00
  float3x2 t = {1,2,3,4,5,6};
#elif TEST7  
  // CHK_TEST7: dx.op.rawBufferStore.f32
  // CHK_TEST7: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00
  // CHK_TEST7: dx.op.rawBufferStore.f32
  // CHK_TEST7: i32 16, i32 undef, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00
  // CHK_TEST7: dx.op.rawBufferStore.f32
  // CHK_TEST7: i32 32, i32 undef, float 9.000000e+00
  float3x3 t = {1,2,3,4,5,6,7,8,9};
#elif TEST8  
  // CHK_TEST8: dx.op.rawBufferStore.f32
  // CHK_TEST8: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00
  // CHK_TEST8: dx.op.rawBufferStore.f32
  // CHK_TEST8: i32 16, i32 undef, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00
  // CHK_TEST8: dx.op.rawBufferStore.f32
  // CHK_TEST8: i32 32, i32 undef, float 9.000000e+00, float 1.000000e+01, float 1.100000e+01, float 1.200000e+01
  float3x4 t = {1,2,3,4,5,6,7,8,9,10,11,12};
#elif TEST9  
  // CHK_TEST9: dx.op.rawBufferStore.f32
  // CHK_TEST9: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00
  // CHK_TEST9: dx.op.rawBufferStore.f32
  // CHK_TEST9: i32 16, i32 undef, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00
  // CHK_TEST9: dx.op.rawBufferStore.f32
  // CHK_TEST9: i32 32, i32 undef, float 9.000000e+00, float 1.000000e+01, float 1.100000e+01, float 1.200000e+01
  float4x3 t = {1,2,3,4,5,6,7,8,9,10,11,12};
#else
  // CHK_TEST10: dx.op.rawBufferStore.f32
  // CHK_TEST10: i32 0, i32 undef, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00
  // CHK_TEST10: dx.op.rawBufferStore.f32
  // CHK_TEST10: i32 16, i32 undef, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00
  // CHK_TEST10: dx.op.rawBufferStore.f32
  // CHK_TEST10: i32 32, i32 undef, float 9.000000e+00, float 1.000000e+01, float 1.100000e+01, float 1.200000e+01
  // CHK_TEST10: dx.op.rawBufferStore.f32
  // CHK_TEST10: i32 48, i32 undef, float 1.300000e+01, float 1.400000e+01, float 1.500000e+01, float 1.600000e+01
	float4x4 t = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
#endif
	buffer.Store(0, t);
}
