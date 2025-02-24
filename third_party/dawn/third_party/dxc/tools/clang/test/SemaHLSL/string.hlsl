// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s
// RUN: %dxc -Tps_6_0 -Wno-unused-value -verify %s

static const string s_global1 = "my global string 1";
/*verify-ast
  VarDecl <col:1, col:33> col:21 used s_global1 'string':'string' static cinit
  `-ImplicitCastExpr <col:33> 'const string' <ArrayToPointerDecay>
    `-StringLiteral <col:33> 'literal string' lvalue "my global string 1"
  */

string s_global2 = "my global string 2";

string s_global3 = s_global1;
/*verify-ast
  VarDecl <col:1, col:20> col:8 s_global3 'string':'string' static cinit
  `-ImplicitCastExpr <col:20> 'string':'string' <LValueToRValue>
    `-DeclRefExpr <col:20> 'string':'string' lvalue Var 's_global1' 'string':'string'
  */

string s_global_concat = "my string " "with "
/*verify-ast
  VarDecl <col:1, line:26:4> line:19:8 s_global_concat 'string':'string' static cinit
  `-ImplicitCastExpr <col:26, line:26:4> 'const string' <ArrayToPointerDecay>
    `-StringLiteral <col:26, line:26:4> 'literal string' lvalue "my string with broken up parts"
  */
   "broken up"
   " parts";

static const bool b1 = s_global1;                           /* expected-error {{cannot initialize a variable of type 'const bool' with an lvalue of type 'string'}} fxc-error {{X3017: cannot implicitly convert from 'const string' to 'const bool'}} */
static const bool b2 = false;
static const string s_global4 = true;                       /* expected-error {{cannot initialize a variable of type 'string' with an rvalue of type 'bool'}} fxc-error {{X3017: cannot implicitly convert from 'bool' to 'const string'}} */
static const string s_global5 = b2;                         /* expected-error {{cannot initialize a variable of type 'string' with an lvalue of type 'const bool'}} fxc-error {{X3017: cannot implicitly convert from 'const bool' to 'const string'}} */

static const int i1 = s_global1;                            /* expected-error {{cannot initialize a variable of type 'const int' with an lvalue of type 'string'}} fxc-error {{X3017: cannot implicitly convert from 'const string' to 'const int'}} */
static const int i2 = 10;
static const string s_global6 = 10;                         /* expected-error {{cannot initialize a variable of type 'string' with an rvalue of type 'literal int'}} fxc-error {{X3017: cannot implicitly convert from 'int' to 'const string'}} */
static const string s_global7 = i2;                         /* expected-error {{cannot initialize a variable of type 'string' with an lvalue of type 'const int'}} fxc-error {{X3017: cannot implicitly convert from 'const int' to 'const string'}} */

static const float f1 = s_global1;                          /* expected-error {{cannot initialize a variable of type 'const float' with an lvalue of type 'string'}} fxc-error {{X3017: cannot implicitly convert from 'const string' to 'const float'}} */
static const float f2 = 3.15;
static const string s_global8= 3.14;                        /* expected-error {{cannot initialize a variable of type 'string' with an rvalue of type 'literal float'}} fxc-error {{X3017: cannot implicitly convert from 'float' to 'const string'}} */
static const string s_global9= f2;                          /* expected-error {{cannot initialize a variable of type 'string' with an lvalue of type 'const float'}} fxc-error {{X3017: cannot implicitly convert from 'const float' to 'const string'}} */

static const string s_global10 = { 'A', 'B', 'C' };         /* expected-error {{too many elements in vector initialization (expected 1 element, have 3)}} fxc-error {{X3017: 's_global10': initializer does not match type}} */
static const string s_global11 = { 1, 2, 3 };               /* expected-error {{too many elements in vector initialization (expected 1 element, have 3)}} fxc-error {{X3017: 's_global11': initializer does not match type}} */
static const string s_global12 = { "ABC" };

string g_strArray[5];                                       /* expected-error {{array of type string is not supported}} fxc-pass {{}} */
vector<string, 4> g_strVector1;                             /* expected-error {{'string' cannot be used as a type parameter where a scalar is required}} fxc-error {{X3122: vector element type must be a scalar type}} */
matrix<string, 4, 4> g_strMatrix1;                          /* expected-error {{'string' cannot be used as a type parameter where a scalar is required}} fxc-error {{X3123: matrix element type must be a scalar type}} */

void hello_here(string message, string s, float f) {        /* expected-error {{parameter of type string is not supported}} expected-error {{parameter of type string is not supported}} fxc-pass {{}} */
  printf(s);
  printf(message);
  printf("%f", f);
}

string get_message() {                                      /* expected-error {{return value of type string is not supported}} fxc-error {{X3038: 'get_message': function return value cannot contain Effects objects}} */
  return "foo";
}

struct test {
  string field;                                             /* expected-error {{string declaration may only appear in global scope}} fxc-pass {{}} */
};

[shader("pixel")]
float4 main() : SV_Target0 {                                /* */
  string str;                                               /* expected-error {{string declaration may only appear in global scope}} fxc-pass {{}} */
  str = s_global2;                                          /* expected-error {{use of undeclared identifier 'str'}} fxc-pass {{}} */

  string strArray[5];                                        /* expected-error {{array of type string is not supported}} fxc-pass {{}} */
  vector<string, 4> strVector1;                             /* expected-error {{'string' cannot be used as a type parameter where a scalar is required}} fxc-error {{X3122: vector element type must be a scalar type}} */
  matrix<string, 4, 4> strMatrix1;                          /* expected-error {{'string' cannot be used as a type parameter where a scalar is required}} fxc-error {{X3123: matrix element type must be a scalar type}} */

  float4 cp4_local;
  printf("hi mom", 1, 2, 3);
  hello_here("a", "b", 1);
  return cp4_local;
}
