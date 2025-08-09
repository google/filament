#version 460
#extension GL_EXT_spirv_intrinsics : enable

spirv_instruction (extensions = ["SPV_KHR_non_semantic_info"], set = "NonSemantic.DebugBreak", id = 1)
void debugBreak();

void main() {
  debugBreak();
}
