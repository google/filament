// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types -HV 2018 %s | FileCheck %s

// CHECK: Use native low precision
// CHECK: cbuffer Foo
// CHECK: {
// CHECK:   struct Foo
// CHECK:   {
// CHECK:       int16_t f_h1;                               ; Offset:    0
// CHECK:       int3 f_f3;                                  ; Offset:    4

// CHECK:       int16_t2 f_h2;                              ; Offset:   16
// CHECK:       int3 f_f3_1;                                ; Offset:   20

// CHECK:       int2 f_f2;                                  ; Offset:   32
// CHECK:       int16_t4 f_h4;                              ; Offset:   40

// CHECK:       int16_t2 f_h2_1;                            ; Offset:   48
// CHECK:       int16_t3 f_h3;                              ; Offset:   52

// CHECK:       double f_d1;                                ; Offset:   64
// CHECK:       int16_t3 f_h3_1;                            ; Offset:   72

// CHECK:       int f_i1;                                   ; Offset:   80
// CHECK:       double f_d2;                                ; Offset:   88
// CHECK:   } Foo;                                          ; Offset:    0 Size:    96
// CHECK: }

cbuffer Foo {
  int16_t f_h1;
  int3 f_f3;

  int16_t2 f_h2;
  int3 f_f3_1;

  int2 f_f2;
  int16_t4 f_h4;

  int16_t2 f_h2_1;
  int16_t3 f_h3;

  double f_d1;
  int16_t3 f_h3_1;

  int   f_i1;
  double f_d2;
}

// CHECK: cbuffer Bar
// CHECK: {
// CHECK:   struct Bar
// CHECK:   {
// CHECK:       int16_t b_h1;                                    ; Offset:    0
// CHECK:       int16_t b_h2;                                    ; Offset:    2
// CHECK:       int16_t b_h3;                                    ; Offset:    4
// CHECK:       int16_t2 b_h4;                                   ; Offset:    6
// CHECK:       int16_t3 b_h5;                                   ; Offset:   10

// CHECK:       int16_t3 b_h6;                                   ; Offset:   16
// CHECK:       int16_t4 b_h7;                                   ; Offset:   22
// CHECK:       int16_t b_h8;                                    ; Offset:   30

// CHECK:       int16_t4 b_h9;                                   ; Offset:   32
// CHECK:       int16_t3 b_h10;                                  ; Offset:   40

// CHECK:       int16_t2 b_h11;                                  ; Offset:   48
// CHECK:       int16_t3 b_h12;                                  ; Offset:   52
// CHECK:       int16_t2 b_h13;                                  ; Offset:   58


// CHECK:       int16_t b_h14;                                   ; Offset:   62
// CHECK:       int16_t b_h16;                                   ; Offset:   64
// CHECK:       int16_t b_h17;                                   ; Offset:   66
// CHECK:       int16_t b_h18;                                   ; Offset:   68
// CHECK:       int16_t b_h19;                                   ; Offset:   70
// CHECK:       int16_t b_h20;                                   ; Offset:   72
// CHECK:       int16_t b_h21;                                   ; Offset:   74
// CHECK:       int16_t b_h22;                                   ; Offset:   76
// CHECK:       int16_t b_h23;                                   ; Offset:   78

// CHECK:   } Bar;                                               ; Offset:    0 Size:    80
// CHECK: }

cbuffer Bar {
  int16_t b_h1;
  int16_t b_h2;
  int16_t b_h3;
  int16_t2 b_h4;
  int16_t3 b_h5;

  int16_t3 b_h6;
  int16_t4 b_h7;
  int16_t b_h8;

  int16_t4 b_h9;
  int16_t3 b_h10;

  int16_t2 b_h11;
  int16_t3 b_h12;
  int16_t2 b_h13;
  int16_t  b_h14;

  int16_t b_h16;
  int16_t b_h17;
  int16_t b_h18;
  int16_t b_h19;
  int16_t b_h20;
  int16_t b_h21;
  int16_t b_h22;
  int16_t b_h23;

}

float4 main() : SV_Target  {
  return f_h1 + f_f3.x
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 1
  + f_h2.x + f_h2.y + f_f3_1.z
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 3
  + f_f2.x + f_h4.x + f_h4.y + f_h4.z + f_h4.w
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
  + f_h2_1.x + f_h2_1.y + f_h3.x + f_h3.y + f_h3.z
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  + f_d1 + f_h3_1.x
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  + f_i1 + f_d2
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 1
  + b_h1 + b_h2 + b_h3 + b_h4.x + b_h5.y + b_h5.x + b_h5.y + b_h5.z +
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
  + b_h6.x + b_h6.y + b_h6.z + b_h7.x + b_h7.y + b_h7.z + b_h7.w + b_h8
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
  + b_h9.x + b_h9.y + b_h9.z + b_h9.w + b_h10.x + b_h10.y + b_h10.z
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  + b_h11.x + b_h11.y + b_h12.x + b_h12.y + b_h12.z + b_h13.x + b_h13.y + b_h14
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  + b_h16 + b_h17 + b_h18 + b_h19 + b_h20 + b_h21 + b_h22 + b_h23;
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
}