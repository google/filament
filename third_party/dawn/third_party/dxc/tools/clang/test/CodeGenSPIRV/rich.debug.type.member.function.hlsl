// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

struct base {
  int p0;

  void f0(float arg) {
    p0 = arg;
  }

  float4 p1;

  int f1(int arg0, float arg1, bool arg2) {
    p0 = arg0;
    p1.y = arg1;
    if (arg2) return arg0;
    return p1.z;
  }

  bool p2;
};

struct foo : base {
  int a;

  void func0(float arg) {
    b.x = arg;
  }

  float4 b;

  int func1(int arg0, float arg1, bool arg2) {
    a = arg0;
    b.y = arg1;
    if (arg2) return arg0;
    return b.z;
  }

  bool c;
};

// CHECK: [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"

// CHECK: [[fooName:%[0-9]+]] = OpString "foo"
// CHECK: [[bool:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Boolean
// CHECK: [[foo:%[0-9]+]] = OpExtInst %void %1 DebugTypeComposite [[fooName]] Structure {{%[0-9]+}} 22 8 {{%[0-9]+}} [[fooName]] %uint_192 FlagIsProtected|FlagIsPrivate
// CHECK: [[float:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Float
// CHECK: [[int:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Signed

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate [[int]] [[foo]] [[int]] [[float]] [[bool]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate %void [[foo]] [[float]]

float4 main(float4 color : COLOR) : SV_TARGET {
  foo a;
  a.func0(1);
  a.func1(1, 1, 1);

  return color;
}
