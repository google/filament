// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

struct foo {
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

// Note:
//    For member functions, the scope in the debug info should be the encapsulating composite.
//    Doing this (as specified in the NonSemantic.Shader.DebugInfo.100) would break the SPIR-V and NonSemantic
//    spec. For this reason, DXC is emitting the wrong scope.
//    See https://github.com/KhronosGroup/SPIRV-Registry/issues/203

// CHECK: [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"

// CHECK: [[bool_name:%[0-9]+]] = OpString "bool"
// CHECK: [[foo:%[0-9]+]] = OpString "foo"
// CHECK: [[c_name:%[0-9]+]] = OpString "c"
// CHECK: [[float_name:%[0-9]+]] = OpString "float"
// CHECK: [[b_name:%[0-9]+]] = OpString "b"
// CHECK: [[int_name:%[0-9]+]] = OpString "int"
// CHECK: [[a_name:%[0-9]+]] = OpString "a"
// CHECK: [[func1:%[0-9]+]] = OpString "foo.func1"
// CHECK: [[func0:%[0-9]+]] = OpString "foo.func0"

// CHECK: [[bool:%[0-9]+]] = OpExtInst %void %1 DebugTypeBasic [[bool_name]] %uint_32 Boolean
// CHECK: [[unit:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit 1 4 {{%[0-9]+}} HLSL

// CHECK-NOT: [[parent:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[foo]] Structure {{%[0-9]+}} 3 8 {{%[0-9]+}} {{%[0-9]+}} %uint_192 FlagIsProtected|FlagIsPrivate [[a:%[0-9]+]] [[b:%[0-9]+]] [[c:%[0-9]+]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK: [[parent:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[foo]] Structure {{%[0-9]+}} 3 8 {{%[0-9]+}} {{%[0-9]+}} %uint_192 FlagIsProtected|FlagIsPrivate [[a:%[0-9]+]] [[b:%[0-9]+]] [[c:%[0-9]+]]

// CHECK: [[c]] = OpExtInst %void [[set]] DebugTypeMember [[c_name]] [[bool]] {{%[0-9]+}} 19 8 [[parent]] %uint_160 %uint_32 FlagIsProtected|FlagIsPrivate
// CHECK: [[float:%[0-9]+]] = OpExtInst %void %1 DebugTypeBasic [[float_name]] %uint_32 Float
// CHECK: [[v4f:%[0-9]+]] = OpExtInst %void %1 DebugTypeVector [[float]] 4
// CHECK: [[b]] = OpExtInst %void [[set]] DebugTypeMember [[b_name]] [[v4f]] {{%[0-9]+}} 10 10 [[parent]] %uint_32 %uint_128 FlagIsProtected|FlagIsPrivate
// CHECK: [[int:%[0-9]+]] = OpExtInst %void %1 DebugTypeBasic [[int_name]] %uint_32 Signed
// CHECK: [[a]] = OpExtInst %void [[set]] DebugTypeMember [[a_name]] [[int]] {{%[0-9]+}} 4 7 [[parent]] %uint_0 %uint_32 FlagIsProtected|FlagIsPrivate

// CHECK-NOT: [[f1:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func1]] {{%[0-9]+}} {{%[0-9]+}} 12 3 [[parent]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 12 %foo_func1
// CHECK-NOT: [[f0:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func0]] {{%[0-9]+}} {{%[0-9]+}} 6 3 [[parent]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 6 %foo_func0
// CHECK: [[f1:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func1]] {{%[0-9]+}} {{%[0-9]+}} 12 3 [[unit]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 12 %foo_func1
// CHECK: [[f0:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func0]] {{%[0-9]+}} {{%[0-9]+}} 6 3 [[unit]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 6 %foo_func0

float4 main(float4 color : COLOR) : SV_TARGET {
  foo a;
  a.func0(1);
  a.func1(1, 1, 1);

  return color;
}
