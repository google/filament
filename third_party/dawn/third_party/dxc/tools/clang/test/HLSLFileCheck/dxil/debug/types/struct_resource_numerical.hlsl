// RUN: %dxc -T vs_6_0 -E main -Zi %s | FileCheck %s

// Test that the debug offset of a numerical type in
// an SROA'd struct is consistent with the reported
// size of a preceding resource-typed field.

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// Texture types are currently 64 bits due to an implementation detail.
// If this changes, this test can be safely updated.
// CHECK-DAG: !DICompositeType(tag: DW_TAG_structure_type, name: "TexAndCoord", {{.*}}, size: 96, align: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "tex", scope: {{.*}}, size: 64, align: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "u", scope: {{.*}}, size: 32, align: 32, offset: 64

// The bit_piece for 'tc.u' should be right after the texture.
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 64, 32)

struct TexAndCoord { Texture1D<float> tex; float u; };

Texture1D<float> g_tex;
float g_u;
float main() : OUT
{
    TexAndCoord tc = { g_tex, g_u };
    return tc.tex.Load(tc.u);
}