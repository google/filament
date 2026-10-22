// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// CHECK: ; Buffer Definitions:
// CHECK: ;
// CHECK: ; cbuffer cbuf
// CHECK: ; {
// CHECK: ;
// CHECK: ;   struct cbuf
// CHECK: ;   {
// CHECK: ;
// CHECK: ;       struct struct.Agg
// CHECK: ;       {
// CHECK: ;
// CHECK: ;           struct struct.Value
// CHECK: ;           {
// CHECK: ;
// CHECK: ;               uint v;                               ; Offset:    0
// CHECK: ;
// CHECK: ;           } value[1];;                              ; Offset:    0
// CHECK: ;
// CHECK: ;           uint fodder;                              ; Offset:    4
// CHECK: ;
// CHECK: ;       } aggie;                                      ; Offset:    0
// CHECK: ;
// CHECK: ;
// CHECK: ;   } cbuf;                                           ; Offset:    0 Size:     8
// CHECK: ;
// CHECK: ; }


// Test Cbuffer validation likely to cause mistaken overlaps
struct Value { uint v; }; // partial stripping will sometimes leave this without annotation

struct Agg {
  Value value[1];
  uint fodder;
};

cbuffer cbuf : register(b1)
{
  Agg aggie;
}

RWBuffer<int> Out : register(u0);

[shader("raygeneration")]
void main()
{
  Out[0] = aggie.value[0].v;
}
