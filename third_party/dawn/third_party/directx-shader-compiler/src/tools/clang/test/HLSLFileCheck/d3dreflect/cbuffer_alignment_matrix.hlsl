// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// CHECK: ; Buffer Definitions:
// CHECK: ;
// CHECK: ; cbuffer cbuf
// CHECK: ; {
// CHECK: ;
// CHECK: ;   struct hostlayout.cbuf
// CHECK: ;   {
// CHECK: ;
// CHECK: ;       struct hostlayout.struct.Agg
// CHECK: ;       {
// CHECK: ;
// CHECK: ;           column_major uint2x2 mat;                 ; Offset:    0
// CHECK: ;           struct struct.Value
// CHECK: ;           {
// CHECK: ;
// CHECK: ;               uint v;                               ; Offset:    32
// CHECK: ;
// CHECK: ;           } value[1];;                              ; Offset:    32
// CHECK: ;
// CHECK: ;           uint fodder;                              ; Offset:    36
// CHECK: ;
// CHECK: ;       } aggie;                                      ; Offset:    0
// CHECK: ;
// CHECK: ;
// CHECK: ;   } cbuf;                                           ; Offset:    0 Size:     40
// CHECK: ;
// CHECK: ; }


// Test Cbuffer validation likely to cause mistaken overlaps
struct Value { uint v; }; // This will be stripped because it has nothing below 32 bits

struct Agg {
  uint2x2 mat; // This will cause the struct to be 
  Value value[1];
  uint fodder;// This will give the struct something to overlap
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
