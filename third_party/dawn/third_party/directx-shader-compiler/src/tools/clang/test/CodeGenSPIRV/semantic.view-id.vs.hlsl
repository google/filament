// RUN: %dxc -T vs_6_1 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability MultiView
// CHECK:      OpExtension "SPV_KHR_multiview"

// CHECK:      OpEntryPoint Vertex
// CHECK-SAME: [[viewindex:%[0-9]+]]

// CHECK:      OpDecorate [[viewindex]] BuiltIn ViewIndex

// CHECK:      [[viewindex]] = OpVariable %_ptr_Input_int Input

int main(int input: SV_ViewID) : A {
    return input;
}

