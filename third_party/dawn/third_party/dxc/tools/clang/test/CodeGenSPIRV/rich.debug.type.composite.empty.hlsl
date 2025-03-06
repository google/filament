// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

struct foo {
};

// CHECK: [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK: [[foo:%[0-9]+]] = OpString "foo"

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeComposite [[foo]] Structure {{%[0-9]+}} 3 8 {{%[0-9]+}} {{%[0-9]+}} %uint_0 FlagIsProtected|FlagIsPrivate

float4 main(float4 color : COLOR) : SV_TARGET {
  foo a;

  return color;
}
