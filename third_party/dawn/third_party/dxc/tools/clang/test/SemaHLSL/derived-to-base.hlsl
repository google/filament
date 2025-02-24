// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tvs_6_0 -verify %s

struct Base {
    float4 a;
    float4 b;
};

struct Derived : Base {
    float4 b;
    float4 c;
};

struct DerivedAgain : Derived {
    float4 c;
    float4 d;
};
[shader("vertex")]
float main() : A {
    DerivedAgain da1, da2;

    (Derived)da1 = (Derived)da2;
    /*verify-ast
      BinaryOperator <col:5, col:29> 'Derived' '='
      |-CStyleCastExpr <col:5, col:14> 'Derived' lvalue <NoOp>
      | `-ImplicitCastExpr <col:14> 'Derived' lvalue <HLSLDerivedToBase (Derived)>
      |   `-DeclRefExpr <col:14> 'DerivedAgain' lvalue Var 'da1' 'DerivedAgain'
      `-ImplicitCastExpr <col:20, col:29> 'Derived' <LValueToRValue>
        `-CStyleCastExpr <col:20, col:29> 'Derived' lvalue <NoOp>
          `-ImplicitCastExpr <col:29> 'Derived' lvalue <HLSLDerivedToBase (Derived)>
            `-DeclRefExpr <col:29> 'DerivedAgain' lvalue Var 'da2' 'DerivedAgain'
    */

    (Base)da1    = (Base)da2;
    /*verify-ast
      BinaryOperator <col:5, col:26> 'Base' '='
      |-CStyleCastExpr <col:5, col:11> 'Base' lvalue <NoOp>
      | `-ImplicitCastExpr <col:11> 'Base' lvalue <HLSLDerivedToBase (Derived -> Base)>
      |   `-DeclRefExpr <col:11> 'DerivedAgain' lvalue Var 'da1' 'DerivedAgain'
      `-ImplicitCastExpr <col:20, col:26> 'Base' <LValueToRValue>
        `-CStyleCastExpr <col:20, col:26> 'Base' lvalue <NoOp>
          `-ImplicitCastExpr <col:26> 'Base' lvalue <HLSLDerivedToBase (Derived -> Base)>
            `-DeclRefExpr <col:26> 'DerivedAgain' lvalue Var 'da2' 'DerivedAgain'
    */

    Derived d;

    (Base)d      = (Base)da2;
    /*verify-ast
      BinaryOperator <col:5, col:26> 'Base' '='
      |-CStyleCastExpr <col:5, col:11> 'Base' lvalue <NoOp>
      | `-ImplicitCastExpr <col:11> 'Base' lvalue <HLSLDerivedToBase (Base)>
      |   `-DeclRefExpr <col:11> 'Derived' lvalue Var 'd' 'Derived'
      `-ImplicitCastExpr <col:20, col:26> 'Base' <LValueToRValue>
        `-CStyleCastExpr <col:20, col:26> 'Base' lvalue <NoOp>
          `-ImplicitCastExpr <col:26> 'Base' lvalue <HLSLDerivedToBase (Derived -> Base)>
            `-DeclRefExpr <col:26> 'DerivedAgain' lvalue Var 'da2' 'DerivedAgain'
    */

    da1          = (DerivedAgain)d; // expected-error {{cannot convert from 'Derived' to 'DerivedAgain'}} fxc-error {{X3017: cannot convert from 'struct Derived' to 'struct DerivedAgain'}}

    return 1.0;
}
