// RUN: %dxc -T vs_6_8 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:                   OpEntryPoint Vertex %main "main"
// CHECK-SAME:              %gl_BaseInstance

// CHECK:                   OpDecorate %gl_BaseInstance BuiltIn BaseInstance

// CHECK: %gl_BaseInstance = OpVariable %_ptr_Input_int Input

int main(int input: SV_StartInstanceLocation) : A {
    return input;
}
