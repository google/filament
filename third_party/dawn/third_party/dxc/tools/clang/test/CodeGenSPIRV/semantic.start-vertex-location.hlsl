// RUN: %dxc -T vs_6_8 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:                   OpEntryPoint Vertex %main "main" [[baseVertex:%[0-9]+]]

// CHECK:                   OpDecorate [[baseVertex]] BuiltIn BaseVertex

// CHECK: [[baseVertex]] = OpVariable %_ptr_Input_int Input

int main(int input: SV_StartVertexLocation) : A {
    return input;
}
