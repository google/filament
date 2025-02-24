// RUN: %dxc -E main -T cs_6_2 -Zi /Od %s | FileCheck %s
// RUN: %dxc -E main -T cs_6_2 -Zi /Od %s -enable-16bit-types | FileCheck %s -check-prefix=CHECK16

struct Foo
{
    min16float3 m_A;
    min16float3 m_B;
};

StructuredBuffer<Foo> buf;
RWTexture1D<float4> out_buf;

[numthreads(1, 1, 1)]
[RootSignature("DescriptorTable(SRV(t0)), DescriptorTable(UAV(u0))")]
void main()
{
    Foo foo = buf[0];
    // foo.m_B.x
    // CHECK-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 96, 16)
    // CHECK16-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 48, 16)

    // foo.m_B.y
    // CHECK-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 128, 16)
    // CHECK16-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 64, 16)

    // foo.m_B.z
    // CHECK-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 160, 16)
    // CHECK16-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 80, 16)

    // foo.m_A.x
    // CHECK-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 0, 16)
    // CHECK16-DAG: call void @llvm.dbg.value(metadata half %{{[0-9]+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"foo" !DIExpression(DW_OP_bit_piece, 0, 16)

    min16float value1 = foo.m_B.x;
    min16float value2 = foo.m_B.y;
    min16float value3 = foo.m_B.z;
    min16float value4 = foo.m_A.x;
    out_buf[0] = float4(value1, value2, value3, value4);
}

// CHECK-DAG: !{{[0-9]+}} = !DICompositeType(tag: DW_TAG_structure_type, name: "Foo", file: !{{[0-9]+}}, line: {{[0-9]+}}, size: 192, align: 32, elements: ![[elements_md:[0-9]+]]
// CHECK-DAG: ![[elements_md]] = !{![[member_md:[0-9]+]]
// CHECK-DAG: ![[member_md]] = !DIDerivedType(tag: DW_TAG_member, name: "m_A", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, baseType: !{{[0-9]+}}, size: 96, align: 32
// CHECK-DAG: !DIBasicType(name: "min16float", size: 16, align: 32, encoding: DW_ATE_float)

// CHECK16-DAG: !{{[0-9]+}} = !DICompositeType(tag: DW_TAG_structure_type, name: "Foo", file: !{{[0-9]+}}, line: {{[0-9]+}}, size: 96, align: 16, elements: ![[elements_md:[0-9]+]]
// CHECK16-DAG: ![[elements_md]] = !{![[member_md:[0-9]+]]
// CHECK16-DAG: ![[member_md]] = !DIDerivedType(tag: DW_TAG_member, name: "m_A", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: {{[0-9]+}}, baseType: !{{[0-9]+}}, size: 48, align: 16
// CHECK16-DAG: !DIBasicType(name: "min16float", size: 16, align: 16, encoding: DW_ATE_float)
