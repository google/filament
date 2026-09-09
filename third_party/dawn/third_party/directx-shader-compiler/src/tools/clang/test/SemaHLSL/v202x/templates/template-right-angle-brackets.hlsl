// RUN: %dxc -T lib_6_3 -HV 202x -verify %s
// RUN: %dxc -T lib_6_3 -HV 202x -ast-dump %s 2>&1 | FileCheck %s

// In HLSL 202x, two consecutive right angle brackets (>>) are allowed in
// template argument lists without spaces, matching C++11 behavior. No
// diagnostic should be emitted for >> or >>> in this mode.

// expected-no-diagnostics

template<typename T>
struct Outer { T val; };

template<typename T>
struct Inner { T val; };

// Nested template type using >> without space: no error in HLSL 202x.
// CHECK: VarDecl {{.*}} g_nested 'Outer<Inner<float> >'
Outer<Inner<float>> g_nested;

template<typename T>
struct Wrapper {
  // CHECK: FieldDecl {{.*}} field 'Outer<Inner<T> >'
  Outer<Inner<T>> field;
};

// Function return type with nested template using >>.
// CHECK: FunctionDecl {{.*}} getVal 'Outer<Inner<int> > ()'
Outer<Inner<int>> getVal();

// Function parameter using >>.
// CHECK: FunctionDecl {{.*}} takeVal 'void (Outer<Inner<int> >)'
void takeVal(Outer<Inner<int>> v);

// Three right angle brackets (>>>) in a template: no error in HLSL 202x.
template<typename T, typename U>
struct Pair { T first; U second; };

// CHECK: VarDecl {{.*}} g_triple 'Outer<Pair<float, Inner<int> > >'
Outer<Pair<float, Inner<int>>> g_triple;
