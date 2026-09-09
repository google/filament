// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: main

class A {
  float m_a;
  void bar() { m_a = 1.2; }
};
class B : A {
  int m_b;
  void bar() {
     A::bar();
  }
  void foo() {
    m_a = 1.3;
    m_b = 3;
  }
};

class C : B {
   void bar() {
      B::bar();
      A::bar();
      m_a = 1.5;
   }
};

float main() : SV_Target {
  C c;
  c.bar();
  c.foo();
  return c.m_a;
}