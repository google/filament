// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:                   OpEntryPoint Vertex %main "main"
// CHECK-SAME:              %gl_VertexIndex

// CHECK:                   OpDecorate %gl_VertexIndex BuiltIn VertexIndex

// CHECK: %gl_VertexIndex = OpVariable %_ptr_Input_int Input

int main(int input: SV_VertexID) : A {
    return input;
}
