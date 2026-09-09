// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: ; Buffer Definitions:
// CHECK: ;
// CHECK: ; cbuffer buf1
// CHECK: ; {
// CHECK: ;
// CHECK: ;   struct buf1
// CHECK: ;   {
// CHECK: ;
// CHECK: ;       struct struct.Foo
// CHECK: ;       {
// CHECK: ;
// CHECK: ;           float4 g1[16];                            ; Offset:    0
// CHECK: ;
// CHECK: ;       } buf1;                                       ; Offset:    0
// CHECK: ;
// CHECK: ;
// CHECK: ;   } buf1;                                           ; Offset:    0 Size:  256
// CHECK: ;
// CHECK: ; }
// CHECK: ;
// CHECK: ; cbuffer buf2
// CHECK: ; {
// CHECK: ;
// CHECK: ;   struct buf2
// CHECK: ;   {
// CHECK: ;
// CHECK: ;       struct struct.Bar
// CHECK: ;       {
// CHECK: ;
// CHECK: ;           struct struct.Foo
// CHECK: ;           {
// CHECK: ;
// CHECK: ;               float4 g1[16];                        ; Offset:    0
// CHECK: ;
// CHECK: ;           } foo;                                    ; Offset:    0
// CHECK: ;
// CHECK: ;           uint3 idx[16];                            ; Offset:  256
// CHECK: ;
// CHECK: ;       } buf2;                                       ; Offset:    0
// CHECK: ;
// CHECK: ;   } buf2;                                           ; Offset:    0 Size: 508
// CHECK: ;
// CHECK: ; }
// CHECK: ;
// CHECK: ;
// CHECK: ; Resource Bindings:
// CHECK: ;
// CHECK: ; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
// CHECK: ; ------------------------------ ---------- ------- ----------- ------- -------------- ------
// CHECK: ; buf1                              cbuffer      NA          NA     CB0    cb77,space3unbounded
// CHECK: ; buf2                              cbuffer      NA          NA     CB1           cb17unbounded

// For cb17
// CHECK:add i32
// CHECK:, 17

// For cb77
// CHECK:add i32
// CHECK:, 77

struct Foo
{
  float4 g1[16];
};

struct Bar
{
  Foo   foo;
  uint3 idx[16];
};

ConstantBuffer<Foo> buf1[] : register(b77, space3);
ConstantBuffer<Bar> buf2[] : register(b17);

float4 main(int3 a : A) : SV_TARGET
{
  return buf1[ buf2[a.x].idx[a.y].z ].g1[a.z + 12].wyyy;
}
