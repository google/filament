// RUN: %dxc -Zi -E main -T ps_6_0 /Zi /O0 %s | FileCheck %s

// CHECK-NOT: !{{[0-9]+}} = !DILocation(line: 0,

// Regression test to make sure debug locations with 0
// as line number don't show up.

struct My_Struct {
  float m_foo;
  float m_bar;
  void set_foo(float foo) {
    m_foo = foo;
    m_foo *= 2;
  }
  void test() {
    m_bar = m_bar * 2;
  }
};

static My_Struct s = { 0.0, 0.0 };

[RootSignature("")]
float main(float foo : FOO) : SV_Target {
  s.set_foo(foo);
  s.test();
  return s.m_bar;
}

