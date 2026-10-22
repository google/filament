// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

// CHECK: @main

// CHECK: !dx.controlflow.hints
// CHECK: !dx.controlflow.hints
// CHECK: !dx.controlflow.hints

// Make sure that even when we don't simplify cfg, DxilValueCache
// is still able to figure out values.

static int g_foo;
static int g_bar;
static int g_baz;

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

Texture2D f(int foo, int bar, int baz) {
  foo += 10;
  if (foo+bar < baz*2)
    return tex0;
  else
    return tex1;
}

[RootSignature("DescriptorTable(SRV(t0, numDescriptors=2))")]
float4 main() : sv_target {
  g_foo = 10;
  [branch]
  if (g_foo > 10)
    g_foo = 30;
  [branch]
  if (g_foo < 50)
    g_foo = 90;
  [branch]
  if (g_foo > 80)
    g_bar = 20;

  g_baz = 30;
  return f(g_foo, g_bar, g_baz).Load(0);
};


