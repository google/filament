// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s

// Tests the printed layout of constant and texture buffers.
// We don't care in what order they get printed

// CHECK: int2 a; ; Offset: 0
// CHECK: int b[2]; ; Offset: 16
// CHECK: int2 c; ; Offset: 36
// CHECK: int2 d; ; Offset: 48
// CHECK: int e; ; Offset: 56
// CHECK: Size: 60

// CHECK: int2 a; ; Offset: 0
// CHECK: int b[2]; ; Offset: 16
// CHECK: int2 c; ; Offset: 36
// CHECK: int2 d; ; Offset: 48
// CHECK: int e; ; Offset: 56
// CHECK: Size: 60

// CHECK: int2 a; ; Offset: 0
// CHECK: int b[2]; ; Offset: 16
// CHECK: int2 c; ; Offset: 36
// CHECK: int2 d; ; Offset: 48
// CHECK: int e; ; Offset: 56
// CHECK: Size: 60

// CHECK: int2 a; ; Offset: 0
// CHECK: int b[2]; ; Offset: 16
// CHECK: int2 c; ; Offset: 36
// CHECK: int2 d; ; Offset: 48
// CHECK: int e; ; Offset: 56
// CHECK: Size: 60

struct Struct
{
    int2 a;
    struct
    {
        int b[2]; // Each element is int4-aligned
        int2 c; // Fits in b[1].yz
        int2 d; // Doesn't fit in b[1].w-, so gets its own int4
    } s;
    int e; // Fits in d.z
};

cbuffer _cbl { Struct cbl; };
ConstantBuffer<Struct> cb;

tbuffer _tbl { Struct tbl; };
TextureBuffer<Struct> tb;

int main() : OUT
{
    return cbl.e + cb.e + tbl.e + tb.e;
}