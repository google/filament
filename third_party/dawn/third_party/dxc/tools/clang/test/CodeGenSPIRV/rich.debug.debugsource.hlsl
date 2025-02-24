// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich-with-source -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[debugSet:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:               {{%[0-9]+}} = OpExtInst %void [[debugSet]] DebugSource {{%[0-9]+}} {{%[0-9]+}}

float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}

