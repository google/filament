// RUN: %dxc -T cs_6_5 -HV 2021 -ast-dump %s | FileCheck %s

struct ConstantsStruct
{
    float Val : Val;
};
ConstantBuffer<ConstantsStruct> Constants;
RWStructuredBuffer<float> Output;

template<typename T>
T Fn(T input)
{
    T ret = input - Constants.Val;

    return ret;
}

[numthreads(64, 1, 1)]
void main()
{
  Output[0] = Fn(1.f);
}

// Verify that the template decl exists
// CHECK:      FunctionTemplateDecl 0x{{[0-9a-fA-F]+}} <line:10:1, line:16:1> line:11:3 Fn
// CHECK-NEXT: TemplateTypeParmDecl 0x{{[0-9a-fA-F]+}} <line:10:10, col:19> col:19 referenced typename T

// Verify that the AST for the template has the Implicit cast as a FlatConversion
// CHECK:      ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:21> 'const ConstantsStruct' lvalue <FlatConversion>

// Verify that the expansion exists for T == float
// CHECK:      FunctionDecl 0x{{[0-9a-fA-F]+}} <line:11:1, line:16:1> line:11:3 used Fn 'float (float)'
// CHECK-NEXT: TemplateArgument type 'float'

// Verify that the cast is again generated as a FlatConversion
// CHECK:      ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:21> 'ConstantsStruct'
// lvalue <FlatConversion>
