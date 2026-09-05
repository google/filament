// RUN: %dxc -E main -T vs_6_2 -DTY1=float -DTY2=float -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float2 -DTY2=float -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float3 -DTY2=float -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float4 -DTY2=float -enable-16bit-types %s | FileCheck %s

// RUN: %dxc -E main -T vs_6_2 -DTY1=float16_t -DTY2=uint -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float16_t2 -DTY2=float16_t -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float16_t3 -DTY2=bool -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float16_t4 -DTY2=min16float -enable-16bit-types %s | FileCheck %s

// RUN: %dxc -E main -T vs_6_2 -DTY1=float -DTY2=float16_t -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float2 -DTY2=int16_t -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float3 -DTY2=bool -enable-16bit-types %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -DTY1=float4 -DTY2=uint16_t -enable-16bit-types %s | FileCheck %s

// RUN: %dxc -E main -T vs_6_2 -DTY1=float4 -DTY2=uint16_t4x4 -enable-16bit-types %s | FileCheck %s -check-prefix=CHECK_ERROR
// RUN: %dxc -E main -T vs_6_2 -DTY1=float4 -DTY2=uint16_t4 -enable-16bit-types %s | FileCheck %s -check-prefix=CHECK_ERROR
// RUN: %dxc -E main -T vs_6_2 -DTY1=float4 -DTY2=float16_t2 -enable-16bit-types %s | FileCheck %s -check-prefix=CHECK_ERROR
// RUN: %dxc -E main -T vs_6_2 -DTY1=uint16_t4x4 -DTY2=float16_t -enable-16bit-types %s | FileCheck %s -check-prefix=CHECK_ERROR

// CHECK: define void @main()
// CHECK_ERROR: note: candidate function not viable: no known conversion from

TY1 main (TY1 a: IN0, TY1 b : IN1, TY2 c : IN2) : OUT {
   return refract(a, b, c);
}