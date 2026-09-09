// RUN: %dxc -T ps_6_6 -HV 2021 -ast-dump %s | FileCheck %s

// This test is a regression test that tests the implementation of L-Value
// conversions work on bitfields, for both constant and structured buffers


struct MyStruct
{
    uint v0: 5;
    uint v1: 15;
    uint v2: 12;
};

ConstantBuffer<MyStruct> myConstBuff;
StructuredBuffer<MyStruct> myStructBuff;

// There should be 4 AST nodes for each access:
/*
1. The HLSL vector element expression (which will be an rvalue)
2. The implicit cast expression for the Vector splat T -> vector<T,1> (which will be an rvalue)
3. The implicit cast expression for the LValueToRValue (which will be an rvalue)
4. The member expression for the bitfield
*/

// temp1 assignment:
// CHECK: -HLSLVectorElementExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'vector<uint, 4>':'vector<unsigned int, 4>' xxxx
// CHECK: -ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'vector<uint, 1>':'vector<unsigned int, 1>' <HLSLVectorSplat>
// CHECK: -ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'const uint':'const unsigned int' <LValueToRValue>
// CHECK: -MemberExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'const uint':'const unsigned int' lvalue bitfield .v2 {{0x[0-9a-fA-F]+}} 

// temp2 assignment:
// CHECK: -HLSLVectorElementExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'vector<uint, 4>':'vector<unsigned int, 4>' xxxx
// CHECK: -ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'vector<uint, 1>':'vector<unsigned int, 1>' <HLSLVectorSplat>
// CHECK: -ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'const uint':'const unsigned int' <LValueToRValue>
// CHECK: -MemberExpr {{0x[0-9a-fA-F]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'const uint':'const unsigned int' lvalue bitfield .v2 {{0x[0-9a-fA-F]+}} 

[RootSignature("RootFlags(0), DescriptorTable(CBV(b0), SRV(t0))")]
float4 main(uint addr: TEXCOORD): SV_Target
{
    float4 temp1 = myStructBuff[0].v2.xxxx;
    float4 temp2 = myConstBuff.v2.xxxx;
    return  temp1 + temp2;
}
