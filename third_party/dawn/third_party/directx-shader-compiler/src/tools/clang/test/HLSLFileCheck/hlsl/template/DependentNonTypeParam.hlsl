// RUN: %dxc -E main -T ps_6_0 -ast-dump -HV 2021 %s | FileCheck -check-prefix=AST %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s



template<uint VSize, typename T>
vector<T, VSize> make_vec(T X) {
  return (vector<T, VSize>)X;
}

// Just verify that we got to codegen, generated main, and load the two used
// components from the input. The actual code-gen is less interesting to this
// test case.

// CHECK: define void @main()
// CHECK: @dx.op.loadInput.f32
// CHECK: @dx.op.loadInput.f32
float2 main(float4 a:A) : SV_Target {
  float3 M4 = make_vec<4>(a.x).xyz;
  return M4.xy * a.y;
}

// AST:      | |-NonTypeTemplateParmDecl 0x{{[0-9a-zA-Z]+}} <line:6:10, col:15> col:15 referenced 'uint':'unsigned int' VSize
// AST-NEXT: | |-TemplateTypeParmDecl 0x{{[0-9a-zA-Z]+}} <col:22, col:31> col:31 referenced typename T
// AST-NEXT: | |-FunctionDecl 0x{{[0-9a-zA-Z]+}} <line:7:1, line:9:1> line:7:18 make_vec 'vector<T, VSize> (T)'
// AST-NEXT: | | |-ParmVarDecl 0x{{[0-9a-zA-Z]+}} <col:27, col:29> col:29 referenced X 'T'
// AST-NEXT: | | `-CompoundStmt 0x{{[0-9a-zA-Z]+}} <col:32, line:9:1>
// AST-NEXT: | |   `-ReturnStmt 0x{{[0-9a-zA-Z]+}} <line:8:3, col:28>
// AST-NEXT: | |     `-CStyleCastExpr 0x{{[0-9a-zA-Z]+}} <col:10, col:28> 'vector<T, VSize>' <Dependent>
// AST-NEXT: | |       `-DeclRefExpr 0x{{[0-9a-zA-Z]+}} <col:28> 'T' lvalue ParmVar 0x{{[0-9a-zA-Z]+}} 'X' 'T'
// AST-NEXT: | `-FunctionDecl 0x{{[0-9a-zA-Z]+}} <line:7:1, line:9:1> line:7:18 used make_vec 'vector<float, 4U> (float)'
// AST-NEXT: |   |-TemplateArgument integral 4
// AST-NEXT: |   |-TemplateArgument type 'float'
// AST-NEXT: |   |-ParmVarDecl 0x{{[0-9a-zA-Z]+}} <col:27, col:29> col:29 used X 'float':'float'
// AST-NEXT: |   `-CompoundStmt 0x{{[0-9a-zA-Z]+}} <col:32, line:9:1>
// AST-NEXT: |     `-ReturnStmt 0x{{[0-9a-zA-Z]+}} <line:8:3, col:28>
// AST-NEXT: |       `-CStyleCastExpr 0x{{[0-9a-zA-Z]+}} <col:10, col:28> 'vector<float, 4U>':'vector<float, 4>' <NoOp>
// AST-NEXT: |         `-ImplicitCastExpr 0x{{[0-9a-zA-Z]+}} <col:28> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
// AST-NEXT: |           `-ImplicitCastExpr 0x{{[0-9a-zA-Z]+}} <col:28> 'float':'float' <LValueToRValue>
// AST-NEXT: |             `-DeclRefExpr 0x{{[0-9a-zA-Z]+}} <col:28> 'float':'float' lvalue ParmVar 0x{{[0-9a-zA-Z]+}} 'X' 'float':'float'
