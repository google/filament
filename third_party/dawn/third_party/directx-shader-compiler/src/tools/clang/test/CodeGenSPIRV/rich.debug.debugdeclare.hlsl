// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// TODO: FlagIsPublic is shown as FlagIsProtected|FlagIsPrivate.

// CHECK: [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK: [[y:%[0-9]+]] = OpExtInst %void [[set]] DebugLocalVariable {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 23 23 {{%[0-9]+}} FlagIsLocal 2
// CHECK: [[x:%[0-9]+]] = OpExtInst %void [[set]] DebugLocalVariable {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 23 14 {{%[0-9]+}} FlagIsLocal 1
// CHECK: [[condition:%[0-9]+]] = OpExtInst %void [[set]] DebugLocalVariable {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 30 8 {{%[0-9]+}} FlagIsLocal
// CHECK: [[expr:%[0-9]+]] = OpExtInst %void [[set]] DebugExpression
// CHECK: [[color:%[0-9]+]] = OpExtInst %void [[set]] DebugLocalVariable {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} 28 20 {{%[0-9]+}} FlagIsLocal 1

// CHECK:     %color = OpFunctionParameter
// CHECK:   {{%[0-9]+}} = OpExtInst %void [[set]] DebugDeclare [[color]] %color [[expr]]
// CHECK: %condition = OpVariable
// CHECK:              OpStore %condition %false
// CHECK:   {{%[0-9]+}} = OpExtInst %void [[set]] DebugDeclare [[condition]] %condition [[expr]]

// CHECK:       %x = OpFunctionParameter
// CHECK:       %y = OpFunctionParameter
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugDeclare [[x]] %x [[expr]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugDeclare [[y]] %y [[expr]]

void foo(int x, float y)
{
  x = x + y;
}

float4 main(float4 color : COLOR) : SV_TARGET
{
  bool condition = false;
  foo(1, color.x);
  return color;
}

