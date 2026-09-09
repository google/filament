// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s

// CHECK: Use native low precision
// CHECK:   struct struct.Foo
// CHECK:   {
// CHECK:       half h1;                                    ; Offset:    0
// CHECK:       float3 f3;                                  ; Offset:    4

// CHECK:       half2 h2;                                   ; Offset:   16
// CHECK:       float3 f3_1;                                ; Offset:   20

// CHECK:       float2 f2;                                  ; Offset:   32
// CHECK:       half4 h4;                                   ; Offset:   40

// CHECK:       half2 h2_1;                                 ; Offset:   48
// CHECK:       half3 h3;                                   ; Offset:   52

// CHECK:       double d1;                                  ; Offset:   64
// CHECK:       half3 h3_1;                                 ; Offset:   72

// CHECK:       int i1;                                     ; Offset:   80
// CHECK:       double d2;                                  ; Offset:   88

// CHECK:   } f;                                            ; Offset:    0 Size:    96

struct Foo {
  half h1;
  float3 f3;

  half2 h2;
  float3 f3_1;

  float2 f2;
  half4 h4;

  half2 h2_1;
  half3 h3;

  double d1;
  half3 h3_1;

  int i1;
  double d2;
};


// CHECK:   struct struct.Bar
// CHECK:   {
// CHECK:       half h1;                                    ; Offset:    0
// CHECK:       half h2;                                    ; Offset:    2
// CHECK:       half h3;                                    ; Offset:    4
// CHECK:       half2 h4;                                   ; Offset:    6
// CHECK:       half3 h5;                                   ; Offset:   10

// CHECK:       half3 h6;                                   ; Offset:   16
// CHECK:       half4 h7;                                   ; Offset:   22
// CHECK:       half h8;                                    ; Offset:   30

// CHECK:       half4 h9;                                   ; Offset:   32
// CHECK:       half3 h10;                                  ; Offset:   40

// CHECK:       half2 h11;                                  ; Offset:   48
// CHECK:       half3 h12;                                  ; Offset:   52
// CHECK:       half2 h13;                                  ; Offset:   58


// CHECK:       half h14;                                   ; Offset:   62
// CHECK:       half h16;                                   ; Offset:   64
// CHECK:       half h17;                                   ; Offset:   66
// CHECK:       half h18;                                   ; Offset:   68
// CHECK:       half h19;                                   ; Offset:   70
// CHECK:       half h20;                                   ; Offset:   72
// CHECK:       half h21;                                   ; Offset:   74
// CHECK:       half h22;                                   ; Offset:   76
// CHECK:       half h23;                                   ; Offset:   78

// CHECK:   } b;                                            ; Offset:    0 Size:    80

struct Bar {
  half h1;
  half h2;
  half h3;
  half2 h4;
  half3 h5;

  half3 h6;
  half4 h7;
  half h8;

  half4 h9;
  half3 h10;

  half2 h11;
  half3 h12;
  half2 h13;
  half  h14;

  half h16;
  half h17;
  half h18;
  half h19;
  half h20;
  half h21;
  half h22;
  half h23;

};

ConstantBuffer<Foo> f : register(b0);
ConstantBuffer<Bar> b : register(b1);

// CHECK: %dx.types.CBufRet.f16.8 = type { half, half, half, half, half, half, half, half }


float4 main() : SV_Target  {
  return f.h1 + f.f3.x
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %f_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %f_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 1
  + f.h2.x + f.h2.y + f.f3_1.z
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %f_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %f_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 3
  + f.f2.x + f.h4.x + f.h4.y + f.h4.z + f.h4.w
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %f_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %f_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
  + f.h2_1.x + f.h2_1.y + f.h3.x + f.h3.y + f.h3.z
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %f_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  + f.d1 + f.h3_1.x
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %f_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %f_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  + f.i1 + f.d2
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %f_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %f_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 1
  + b.h1 + b.h2 + b.h3 + b.h4.x + b.h5.y + b.h5.x + b.h5.y + b.h5.z +
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %b_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
  + b.h6.x + b.h6.y + b.h6.z + b.h7.x + b.h7.y + b.h7.z + b.h7.w + b.h8
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %b_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
  + b.h9.x + b.h9.y + b.h9.z + b.h9.w + b.h10.x + b.h10.y + b.h10.z
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %b_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  + b.h11.x + b.h11.y + b.h12.x + b.h12.y + b.h12.z + b.h13.x + b.h13.y + b.h14
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %b_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  + b.h16 + b.h17 + b.h18 + b.h19 + b.h20 + b.h21 + b.h22 + b.h23;
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %b_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
}