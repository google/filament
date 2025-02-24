// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Regression test for a crash that occured when a memcpy replacement
// resulted in a global operand to a bitcast converting between inherited types
// The texture member prevents more trivial casting
// The three-level inheritance results in two bitcasts, one of which is the problem one

// CHECK: @main
struct A
{
  float4 stuff;
};

struct B : A
{
  Texture2D m_texture;
  float4 gimme() {return stuff;}
};

struct C : B
{
  void dostuff() {stuff = 0;}
  static C New(float4 f) { C c; c.stuff = f; return c; }
};

static const C globby = C::New(float4(1,2,3,4));

float4 f(C classy) {
  return classy.gimme();
}
float4 main() : SV_Target
{
  C loki = globby;
  return f(loki);
}

