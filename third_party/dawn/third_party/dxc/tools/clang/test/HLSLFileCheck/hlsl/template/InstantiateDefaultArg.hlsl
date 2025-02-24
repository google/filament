// RUN: %dxc -T vs_6_6 -HV 2021 %s -ast-dump | FileCheck %s -check-prefix=AST
// RUN: %dxc -T vs_6_6 -HV 2021 %s -fcgl | FileCheck %s

template<typename T>
T test(const T a, const T b = 0)
{
  return a + b;
}

// This test verifies a few different modes if instantiation for default
// arguments.

// The first chunk of AST being verified is the FunctionTemplateDecl. In the
// template we have boh parameters defined and the second has a template pattern
// for the `b` parameter.

// AST: FunctionTemplateDecl {{.*}} test
// AST-NEXT: TemplateTypeParmDecl {{.*}} referenced typename T
// AST-NEXT: FunctionDecl {{.*}} test 'T (const T, const T)'
// AST-NEXT: ParmVarDecl {{.*}} referenced a 'const T'
// AST-NEXT: ParmVarDecl {{.*}} referenced b 'const T' cinit
// AST-NEXT: IntegerLiteral {{.*}} 'literal int' 0
// AST-NEXT: CompoundStmt
// AST-NEXT: ReturnStmt
// AST-NEXT: BinaryOperator {{.*}} 'const T' '+'
// AST-NEXT: DeclRefExpr {{.*}} 'const T' lvalue ParmVar {{.*}} 'a' 'const T'
// AST-NEXT: DeclRefExpr {{.*}} 'const T' lvalue ParmVar {{.*}} 'b' 'const T'

// The second AST block is the test<float> instantiation. It has a full
// instantiation including instantiation of the default template parameter. This
// instantiation is called both with and without the default argument.

// AST: FunctionDecl {{.*}} used test 'float (const float, const float)'
// AST-NEXT: TemplateArgument type 'float'
// AST-NEXT: ParmVarDecl {{.*}} used a 'const float':'const float'
// AST-NEXT: ParmVarDecl {{.*}} used b 'const float':'const float' cinit
// AST-NEXT: ImplicitCastExpr {{.*}} 'float':'float' <IntegralToFloating>
// AST-NEXT: IntegerLiteral {{.*}} 'literal int' 0
// AST-NEXT: CompoundStmt

// The third AST block is the test<int> instantiation. This instantiation is
// is also a full instantiation including the initializer for the `b` parameter.
// This function is only called with the default argument used.

// AST: FunctionDecl {{.*}} used test 'int (const int, const int)'
// AST-NEXT: TemplateArgument type 'int'
// AST-NEXT: ParmVarDecl {{.*}} used a 'const int':'const int'
// AST-NEXT: ParmVarDecl {{.*}} used b 'const int':'const int' cinit
// AST-NEXT: ImplicitCastExpr {{.*}} 'int':'int' <IntegralCast>
// AST-NEXT: IntegerLiteral {{.*}} 'literal int' 0
// AST-NEXT: CompoundStmt

// The final AST block is the test<double> instantation. This instantiation does
// not include instantiation of the default parameter. The default parameter is
// only instantiated when used, and this instantation is only called providing
// both arguments.

// AST: FunctionDecl {{.*}} used test 'double (const double, const double)'
// AST-NEXT: TemplateArgument type 'double'
// AST-NEXT: ParmVarDecl {{.*}}  used a 'const double':'const double'
// AST-NEXT: ParmVarDecl {{.*}} used b 'const double':'const double'
// AST-NEXT: CompoundStmt


float4 main(uint vertex_id : SV_VertexID) : SV_Position
{
  return float4(test<float>(1, 2), test<float>(4), (float)test<int>(5),
                (float)test<double>(6, 7));
}

// CHECK: call float @"\01??$test{{.*}}"(float 1.000000e+00, float 2.000000e+00)
// CHECK: call float @"\01??$test{{.*}}"(float 4.000000e+00, float 0.000000e+00)
// CHECK: call i32 @"\01??$test{{.*}}"(i32 5, i32 0)
// CHECK: call double @"\01??$test{{.*}}"(double 6.000000e+00, double 7.000000e+00)
