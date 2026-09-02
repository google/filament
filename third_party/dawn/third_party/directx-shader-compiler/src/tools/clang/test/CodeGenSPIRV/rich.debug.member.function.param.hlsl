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

// CHECK: [[fooName:%[0-9]+]] = OpString "foo"
// CHECK: [[func1:%[0-9]+]] = OpString "foo.func1"
// CHECK: [[arg2:%[0-9]+]] = OpString "arg2"
// CHECK: [[arg1:%[0-9]+]] = OpString "arg1"
// CHECK: [[arg0:%[0-9]+]] = OpString "arg0"
// CHECK: [[this:%[0-9]+]] = OpString "this"
// CHECK: [[func0:%[0-9]+]] = OpString "foo.func0"
// CHECK: [[arg:%[0-9]+]] = OpString "arg"

// CHECK: [[bool:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Boolean
// CHECK: [[unit:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit 1 4 {{%[0-9]+}} HLSL
// CHECK: [[foo:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[fooName]] Structure {{%[0-9]+}} 3 8

// CHECK: [[float:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Float
// CHECK: [[int:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Signed

// CHECK: [[func1Type:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate [[int]] [[foo]] [[int]] [[float]] [[bool]]

// CHECK-NOT: [[f1:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func1]] [[func1Type]] {{%[0-9]+}} 12 3 [[foo]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 12 %foo_func1
// CHECK: [[f1:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func1]] [[func1Type]] {{%[0-9]+}} 12 3 [[unit]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 12 %foo_func1

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[arg2]] {{%[0-9]+}} {{%[0-9]+}} 12 40 [[f1]] FlagIsLocal 4
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[arg1]] {{%[0-9]+}} {{%[0-9]+}} 12 29 [[f1]] FlagIsLocal 3
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[arg0]] {{%[0-9]+}} {{%[0-9]+}} 12 17 [[f1]] FlagIsLocal 2
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[this]] [[foo]] {{%[0-9]+}} 12 3 [[f1]] FlagArtificial|FlagObjectPointer 1
// CHECK: [[func0Type:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate %void [[foo]] [[float]]

// CHECK-NOT: [[f0:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func0]] [[func0Type]] {{%[0-9]+}} 6 3 [[foo]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 6 %foo_func0
// CHECK: [[f0:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[func0]] [[func0Type]] {{%[0-9]+}} 6 3 [[unit]] {{%[0-9]+}} FlagIsProtected|FlagIsPrivate 6 %foo_func0

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[arg]] {{%[0-9]+}} {{%[0-9]+}} 6 20 [[f0]] FlagIsLocal 2
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugLocalVariable [[this]] [[foo]] {{%[0-9]+}} 6 3 [[f0]] FlagArtificial|FlagObjectPointer 1

float4 main(float4 color : COLOR) : SV_TARGET {
  foo a;
  a.func0(1);
  a.func1(1, 1, 1);

  return color;
}
