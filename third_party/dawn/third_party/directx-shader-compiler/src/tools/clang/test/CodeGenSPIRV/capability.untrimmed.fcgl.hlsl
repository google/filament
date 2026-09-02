// RUN: %dxc -T cs_6_0 -E main -fcgl %s -spirv | FileCheck %s

// DXC generates by default all capabilities, and calls a specific pass
// to trim unwanted capabilities. This is done to prevent code duplication
// between the optimizer, and DXC.
// -fcgl is used for debug purposed, and disable all passes, even
// legalization. So if -fcgl is called, the capability should be present,
// event if unused.

// CHECK: OpCapability MinLod
// CHECK: OpCapability FragmentShaderSampleInterlockEXT
// CHECK: OpCapability FragmentShaderPixelInterlockEXT
// CHECK: OpCapability FragmentShaderShadingRateInterlockEXT

// CHECK: OpExtension "SPV_EXT_fragment_shader_interlock"
[numthreads(1, 1, 1)]
void main() { }
