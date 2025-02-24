// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types -HV 2018 %s | FileCheck %s

// CHECK: Use native low precision
// CHECK:   struct struct.Foo
// CHECK:   {
// CHECK:       int16_t h1;                               ; Offset:    0
// CHECK:       int3 f3;                                  ; Offset:    4

// CHECK:       int16_t2 h2;                              ; Offset:   16
// CHECK:       int3 f3_1;                                ; Offset:   20

// CHECK:       int2 f2;                                  ; Offset:   32
// CHECK:       int16_t4 h4;                              ; Offset:   40

// CHECK:       int16_t2 h2_1;                            ; Offset:   48
// CHECK:       int16_t3 h3;                              ; Offset:   52

// CHECK:       double d1;                                ; Offset:   64
// CHECK:       int16_t3 h3_1;                            ; Offset:   72

// CHECK:       int i1;                                   ; Offset:   80
// CHECK:       double d2;                                ; Offset:   88

// CHECK:   } f;                                          ; Offset:    0 Size:    96

struct Foo {
  int16_t h1;
  int3 f3;

  int16_t2 h2;
  int3 f3_1;

  int2 f2;
  int16_t4 h4;

  int16_t2 h2_1;
  int16_t3 h3;

  double d1;
  int16_t3 h3_1;

  int    i1;
  double d2;
};


// CHECK:   struct struct.Bar
// CHECK:   {
// CHECK:       int16_t h1;                                    ; Offset:    0
// CHECK:       uint16_t h2;                                   ; Offset:    2
// CHECK:       int16_t h3;                                    ; Offset:    4
// CHECK:       uint16_t2 h4;                                  ; Offset:    6
// CHECK:       int16_t3 h5;                                   ; Offset:   10

// CHECK:       uint16_t3 h6;                                  ; Offset:   16
// CHECK:       int16_t4 h7;                                   ; Offset:   22
// CHECK:       int16_t h8;                                    ; Offset:   30

// CHECK:       uint16_t4 h9;                                  ; Offset:   32
// CHECK:       int16_t3 h10;                                  ; Offset:   40

// CHECK:       uint16_t2 h11;                                 ; Offset:   48
// CHECK:       int16_t3 h12;                                  ; Offset:   52
// CHECK:       int16_t2 h13;                                  ; Offset:   58


// CHECK:       uint16_t h14;                                  ; Offset:   62
// CHECK:       int16_t h16;                                   ; Offset:   64
// CHECK:       int16_t h17;                                   ; Offset:   66
// CHECK:       uint16_t h18;                                  ; Offset:   68
// CHECK:       int16_t h19;                                   ; Offset:   70
// CHECK:       int16_t h20;                                   ; Offset:   72
// CHECK:       int16_t h21;                                   ; Offset:   74
// CHECK:       uint16_t h22;                                  ; Offset:   76
// CHECK:       int16_t h23;                                   ; Offset:   78

// CHECK:   } b;                                            ; Offset:    0 Size:    80

struct Bar {
  int16_t h1;
  uint16_t h2;
  int16_t h3;
  uint16_t2 h4;
  int16_t3 h5;

  uint16_t3 h6;
  int16_t4 h7;
  int16_t h8;

  uint16_t4 h9;
  int16_t3 h10;

  uint16_t2 h11;
  int16_t3 h12;
  int16_t2 h13;
  uint16_t  h14;

  int16_t h16;
  int16_t h17;
  uint16_t h18;
  int16_t h19;
  int16_t h20;
  int16_t h21;
  uint16_t h22;
  int16_t h23;

};

ConstantBuffer<Foo> f : register(b0);
ConstantBuffer<Bar> b : register(b1);

// CHECK: %dx.types.CBufRet.i16.8 = type { i16, i16, i16, i16, i16, i16, i16, i16 }

int4 main() : SV_Target  {
  return f.h1 + f.f3.x
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %f_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %f_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 1
  + f.h2.x + f.h2.y + f.f3_1.z
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %f_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %f_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 3
  + f.f2.x + f.h4.x + f.h4.y + f.h4.z + f.h4.w
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %f_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %f_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
  + f.h2_1.x + f.h2_1.y + f.h3.x + f.h3.y + f.h3.z
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %f_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  + f.d1 + f.h3_1.x
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %f_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %f_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  + f.i1 + f.d2
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %f_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %f_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 1
  + b.h1 + b.h2 + b.h3 + b.h4.x + b.h5.y + b.h5.x + b.h5.y + b.h5.z +
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %b_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
  + b.h6.x + b.h6.y + b.h6.z + b.h7.x + b.h7.y + b.h7.z + b.h7.w + b.h8
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %b_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
  + b.h9.x + b.h9.y + b.h9.z + b.h9.w + b.h10.x + b.h10.y + b.h10.z
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %b_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  + b.h11.x + b.h11.y + b.h12.x + b.h12.y + b.h12.z + b.h13.x + b.h13.y + b.h14
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %b_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  + b.h16 + b.h17 + b.h18 + b.h19 + b.h20 + b.h21 + b.h22 + b.h23;
  // CHECK: call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %b_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.i16.8 {{%[0-9]+}}, 7
}