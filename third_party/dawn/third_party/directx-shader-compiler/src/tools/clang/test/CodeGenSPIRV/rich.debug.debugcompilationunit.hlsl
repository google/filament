// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[debugSet:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:   [[debugSource:%[0-9]+]] = OpExtInst %void [[debugSet]] DebugSource
// CHECK:               {{%[0-9]+}} = OpExtInst %void [[debugSet]] DebugCompilationUnit 1 4 [[debugSource]] HLSL

float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}

