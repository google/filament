// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Check correct assignment of semantic indices to parameter with array of
// struct followed by another argument, which was triggering an assert
// and invalid indexing for updating the index in the annotation.

// CHECK: ; Output signature:
// CHECK-NEXT: ;
// CHECK-NEXT: ; Name                 Index   Mask Register SysValue  Format   Used
// CHECK-NEXT: ; -------------------- ----- ------ -------- -------- ------- ------
// Out.foo[0].a
// CHECK-NEXT: ; OUT                      0   x           0     NONE   float   x
// Out.foo[0].b[0]
// CHECK-NEXT: ; OUT                      1    y          0     NONE   float    y
// Out.foo[1].a
// CHECK-NEXT: ; OUT                      3   x           1     NONE   float   x
// Out.foo[0].b[1]
// CHECK-NEXT: ; OUT                      2    y          1     NONE   float    y
// Out.foo[2].a
// CHECK-NEXT: ; OUT                      6   x           2     NONE   float   x
// Out.foo[1].b[0]
// CHECK-NEXT: ; OUT                      4    y          2     NONE   float    y
// Out.foo[1].b[1]
// CHECK-NEXT: ; OUT                      5    y          3     NONE   float    y
// Out.foo[2].b[0]
// CHECK-NEXT: ; OUT                      7    y          4     NONE   float    y
// Out.foo[2].b[1]
// CHECK-NEXT: ; OUT                      8    y          5     NONE   float    y
// Out.pos
// CHECK-NEXT: ; OUT                      9   xyzw        6     NONE   float   xyzw

// CHECK: !dx.entryPoints = !{![[main:[0-9]+]]}
// CHECK: ![[main]] = !{void ()* @main, !"main", ![[signatures:[0-9]+]], null, null}
// CHECK: ![[signatures]] = !{!{{[0-9]+}}, ![[outSig:[0-9]+]], null}
// CHECK: ![[m3_15:[0-9]+]] = !{i32 3, i32 15}
// CHECK: ![[m3_1:[0-9]+]] = !{i32 3, i32 1}
// CHECK: ![[outSig]] = !{![[foo_a:[0-9]+]], ![[foo_b:[0-9]+]], ![[out_pos:[0-9]+]]}
// CHECK: ![[foo_a]] = !{i32 0, !"OUT", i8 9, i8 0, ![[sem_idx_a:[0-9]+]], i8 2, i32 3, i8 1, i32 0, i8 0, ![[m3_1]]}
// CHECK: ![[sem_idx_a]] = !{i32 0, i32 3, i32 6}
// CHECK: ![[foo_b]] = !{i32 1, !"OUT", i8 9, i8 0, ![[sem_idx_b:[0-9]+]], i8 2, i32 6, i8 1, i32 0, i8 1, ![[m3_1]]}
// CHECK: ![[sem_idx_b]] = !{i32 1, i32 2, i32 4, i32 5, i32 7, i32 8}
// CHECK: ![[out_pos]] = !{i32 2, !"OUT", i8 9, i8 0, ![[sem_idx_pos:[0-9]+]], i8 2, i32 1, i8 4, i32 6, i8 0, ![[m3_15]]}
// CHECK: ![[sem_idx_pos]] = !{i32 9}


struct Foo {
    float a : A;
    float b[2] : B;
};
struct INPUT {
    float4 pos : SV_Position;
    Foo foo[3] : FOO;
};
struct OUTPUT {
    Foo foo[3] : FOO;
    float4 pos : SV_Position;
};

void main(INPUT In : IN, out OUTPUT Out : OUT) {
    Out.pos = In.pos;
    Out.foo = In.foo;
}
