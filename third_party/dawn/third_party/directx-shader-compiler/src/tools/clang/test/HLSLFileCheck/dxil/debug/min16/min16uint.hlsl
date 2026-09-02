// RUN: %dxc -E main -T cs_6_2 -Zi -Od %s | FileCheck %s
// RUN: %dxc -E main -T cs_6_2 -Zi -Od %s -enable-16bit-types | FileCheck %s -check-prefix=CHECK16

// CHECK-DAG: call void @llvm.dbg.value(metadata i16 1, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 0, 16)
// CHECK-DAG: call void @llvm.dbg.value(metadata i16 2, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 32, 16)

// CHECK16-DAG: call void @llvm.dbg.value(metadata i16 1, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 0, 16)
// CHECK16-DAG: call void @llvm.dbg.value(metadata i16 2, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 16, 16)

struct Foo
{
    min16uint m_A;
    min16uint m_B;
};
RWTexture1D<uint2> out_buf;

[numthreads(1, 1, 1)]
[RootSignature("")]
[RootSignature("DescriptorTable(SRV(t0)), DescriptorTable(UAV(u0))")]
void main()
{
    Foo foo = { 1, 2 };
    min16uint value1 = foo.m_B;
    min16uint value2 = foo.m_A;
    out_buf[0] = uint2(value1, value2);
}

// CHECK-DAG: !{{[0-9]+}} = !DICompositeType(tag: DW_TAG_structure_type, name: "Foo", file: !{{[0-9]+}}, line: {{[0-9]+}}, size: 64, align: 32, elements: ![[elements_md:[0-9]+]]
// CHECK-DAG: ![[elements_md]] = !{![[member_md:[0-9]+]]
// CHECK-DAG: ![[member_md]] = !DIDerivedType(tag: DW_TAG_member, name: "m_A", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, baseType: !{{[0-9]+}}, size: 16, align: 32
// CHECK-DAG: !DIBasicType(name: "min16uint", size: 16, align: 32, encoding: DW_ATE_unsigned)

// CHECK16-DAG: !{{[0-9]+}} = !DICompositeType(tag: DW_TAG_structure_type, name: "Foo", file: !{{[0-9]+}}, line: {{[0-9]+}}, size: 32, align: 16, elements: ![[elements_md:[0-9]+]]
// CHECK16-DAG: ![[elements_md]] = !{![[member_md:[0-9]+]]
// CHECK16-DAG: ![[member_md]] = !DIDerivedType(tag: DW_TAG_member, name: "m_A", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, baseType: !{{[0-9]+}}, size: 16, align: 16
// CHECK16-DAG: !DIBasicType(name: "min16uint", size: 16, align: 16, encoding: DW_ATE_unsigned)

