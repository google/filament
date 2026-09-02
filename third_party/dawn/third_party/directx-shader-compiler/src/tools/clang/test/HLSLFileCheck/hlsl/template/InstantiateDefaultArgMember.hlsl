// RUN: %dxc -T vs_6_6 -HV 2021 %s -ast-dump | FileCheck %s -check-prefix=AST
// RUN: %dxc -T vs_6_6 -HV 2021 %s -fcgl | FileCheck %s

template<typename T>
struct Pupper {
  T test(const T a, const T b = 0)
  {
    return a + b;
  }
};

// This test verifies a few different modes if instantiation for default
// arguments.

// The first chunk of AST being verified is the ClassTemplateDecl for Pupper. In
// the template we have boh parameters defined and the second has a template
// pattern for the `b` parameter.

// AST: ClassTemplateDecl {{.*}} Pupper
// AST-NEXT: TemplateTypeParmDecl {{.*}} referenced typename T
// AST-NEXT: CXXRecordDecl {{.*}} struct Pupper definition
// AST-NEXT: CXXRecordDecl {{.*}} implicit struct Pupper
// AST-NEXT: CXXMethodDecl {{.*}} test 'T (const T, const T)'
// AST-NEXT: ParmVarDecl {{.*}} referenced a 'const T'
// AST-NEXT: ParmVarDecl {{.*}} referenced b 'const T' cinit
// AST-NEXT: IntegerLiteral {{.*}} 'literal int' 0
// AST-NEXT: CompoundStmt
// AST-NEXT: ReturnStmt
// AST-NEXT: BinaryOperator {{.*}} 'const T' '+'
// AST-NEXT: DeclRefExpr {{.*}} 'const T' lvalue ParmVar {{.*}} 'a' 'const T'
// AST-NEXT: DeclRefExpr {{.*}} 'const T' lvalue ParmVar {{.*}} 'b' 'const T'

// The second AST block is the Pupper<float> instantiation. It has a full
// instantiation including instantiation of the default template parameter. This
// instantiation is called both with and without the default argument.

// AST: ClassTemplateSpecializationDecl {{.*}} struct Pupper definition
// AST-NEXT: TemplateArgument type 'float'
// AST-NEXT: CXXRecordDecl {{.*}} implicit struct Pupper
// AST-NEXT: `-CXXMethodDecl {{.*}} used test 'float (const float, const float)'
// AST-NEXT:   |-ParmVarDecl {{.*}} used a 'const float':'const float'
// AST-NEXT:   |-ParmVarDecl {{.*}} used b 'const float':'const float' cinit
// AST-NEXT:   | `-ImplicitCastExpr {{.*}} 'float':'float' <IntegralToFloating>
// AST-NEXT:   |   `-IntegerLiteral {{.*}} 'literal int' 0
// AST-NEXT:   `-CompoundStmt

// The third AST block is the Pupper<int> instantiation. This instantiation is
// is also a full instantiation including the initializer for the `b` parameter.
// This function is only called with the default argument used.

// AST: ClassTemplateSpecializationDecl {{.*}} struct Pupper definition
// AST-NEXT: |-TemplateArgument type 'int'
// AST-NEXT: |-CXXRecordDecl {{.*}} implicit struct Pupper
// AST-NEXT: `-CXXMethodDecl {{.*}} used test 'int (const int, const int)'
// AST-NEXT:   |-ParmVarDecl {{.*}} used a 'const int':'const int'
// AST-NEXT:   |-ParmVarDecl {{.*}} used b 'const int':'const int' cinit
// AST-NEXT:   | `-ImplicitCastExpr {{.*}} 'int':'int' <IntegralCast>
// AST-NEXT:   |   `-IntegerLiteral {{.*}} 'literal int' 0
// AST-NEXT:   `-CompoundStmt

// The final AST block is the Pupper<double> instantation. This instantiation
// does not include instantiation of the default parameter. The default
// parameter is only instantiated when used, and this instantation is only
// called providing both arguments.

// AST: ClassTemplateSpecializationDecl {{.*}} struct Pupper definition
// AST-NEXT: |-TemplateArgument type 'double'
// AST-NEXT: |-CXXRecordDecl {{.*}} implicit struct Pupper
// AST-NEXT: `-CXXMethodDecl {{.*}} used test 'double (const double, const double)'
// AST-NEXT:   |-ParmVarDecl {{.*}} used a 'const double':'const double'
// AST-NEXT:   |-ParmVarDecl {{.*}} used b 'const double':'const double'
// AST-NEXT:   `-CompoundStmt 

float4 main(uint vertex_id : SV_VertexID) : SV_Position
{
  Pupper<float> PF;
  Pupper<int> PI;
  Pupper<double> PD;
  return float4(PF.test(1, 2), PF.test(4), (float)PI.test(5),
                (float)PD.test(6, 7));
}

// CHECK: call float @"\01?test@?$Pupper@{{.*}}"(%"struct.Pupper<float>"* {{.*}}, float 1.000000e+00, float 2.000000e+00)
// CHECK: call float @"\01?test@?$Pupper@{{.*}}"(%"struct.Pupper<float>"* {{.*}}, float 4.000000e+00, float 0.000000e+00)
// CHECK: call i32 @"\01?test@?$Pupper@{{.*}}"(%"struct.Pupper<int>"* {{.*}}, i32 5, i32 0)
// CHECK: call double @"\01?test@?$Pupper@{{.*}}"(%"struct.Pupper<double>"* {{.*}}, double 6.000000e+00, double 7.000000e+00)
