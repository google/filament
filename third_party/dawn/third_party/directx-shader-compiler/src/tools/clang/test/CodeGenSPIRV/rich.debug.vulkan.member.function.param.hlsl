// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan-with-source -O0 %s -spirv | FileCheck %s

// CHECK-DAG:                   OpExtension "SPV_KHR_non_semantic_info"
// CHECK-DAG:                   OpExtension "SPV_KHR_relaxed_extended_instruction"
// CHECK:     [[set:%[0-9]+]] = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"


// CHECK-DAG:    [[strInt:%[0-9]+]] = OpString "int"
// CHECK-DAG:  [[strFloat:%[0-9]+]] = OpString "float"
// CHECK-DAG:   [[strBool:%[0-9]+]] = OpString "bool"
// CHECK-DAG:      [[strA:%[0-9]+]] = OpString "a"
// CHECK-DAG:      [[strB:%[0-9]+]] = OpString "b"
// CHECK-DAG:      [[strC:%[0-9]+]] = OpString "c"
// CHECK-DAG:  [[strFunc0:%[0-9]+]] = OpString "foo.func0"
// CHECK-DAG:    [[strFoo:%[0-9]+]] = OpString "foo"
// CHECK-DAG:  [[strFunc1:%[0-9]+]] = OpString "foo.func1"


// CHECK:    [[int:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[strInt]] %uint_32 %uint_4 %uint_0
// CHECK:   [[unit:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit %uint_1 %uint_4 {{%[0-9]+}} %uint_5

struct foo {

// CHECK: [[a:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeMember [[strA]] [[int]] {{%[0-9]+}} %uint_25 %uint_7 %uint_0 %uint_32 %uint_3
  int a;

// CHECK:  [[float:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[strFloat]] %uint_32 %uint_3 %uint_0
// CHECK:    [[v4f:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[float]] %uint_4
// CHECK:      [[b:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeMember [[strB]] [[v4f]] {{%[0-9]+}} %uint_30 %uint_10 %uint_32 %uint_128 %uint_3
  float4 b;

// CHECK:   [[bool:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[strBool]] %uint_32 %uint_2 %uint_0
// CHECK:      [[c:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeMember [[strC]] [[bool]] {{%[0-9]+}} %uint_34 %uint_8 %uint_160 %uint_32 %uint_3
  bool c;

  void func0(float arg) {
    b.x = arg;
  }

// CHECK: [[func0t:%[0-9]+]] = OpExtInstWithForwardRefsKHR %void [[set]] DebugTypeFunction %uint_3 %void [[foo:%[0-9]+]] {{%[0-9]+}}
// CHECK:  [[func0:%[0-9]+]] = OpExtInstWithForwardRefsKHR %void [[set]] DebugFunction [[strFunc0]] [[func0t]] {{%[0-9]+}} %uint_36 %uint_3 [[foo]] {{%[0-9]+}} %uint_3 %uint_36
// CHECK:            [[foo]] = OpExtInstWithForwardRefsKHR %void [[set]] DebugTypeComposite [[strFoo]] %uint_1 {{%[0-9]+}} %uint_22 %uint_8 [[unit:%[0-9]+]] [[strFoo]] %uint_192 %uint_3 [[a:%[0-9]+]] [[b:%[0-9]+]] [[c:%[0-9]+]] [[func0]] [[func1:%[0-9]+]]

// CHECK:  [[func1t:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction %uint_3 [[int]] [[foo:%[0-9]+]] [[int]] [[float]] [[bool]]
// CHECK:   [[func1:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[strFunc1]] [[func1t]] {{%[0-9]+}} %uint_46 %uint_3 [[foo]] {{%[0-9]+}} %uint_3 %uint_46
  int func1(int arg0, float arg1, bool arg2) {
    a = arg0;
    b.y = arg1;
    if (arg2) return arg0;
    return b.z;
  }

};


float4 main(float4 color : COLOR) : SV_TARGET {
  foo a;
  a.func0(1);
  a.func1(1, 1, 1);

  return color;
}
