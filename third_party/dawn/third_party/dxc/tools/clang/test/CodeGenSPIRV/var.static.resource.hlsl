// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// No inline initializer

// CHECK: %sRWTex = OpVariable %_ptr_Private_type_2d_image Private
// CHECK: %lRWTex = OpVariable %_ptr_Private_type_2d_image Private

static RWTexture2D<float4> sRWTex;

// No OpStore

// CHECK-LABEL:    %main = OpFunction
// CHECK-NEXT:             OpLabel
// CHECK-NEXT:             OpFunctionCall %void %src_main
// CHECK-NEXT:             OpReturn
// CHECK-NEXT:             OpFunctionEnd

// CHECK:     %src_main = OpFunction
// CHECK-NEXT:             OpLabel
// CHECK-NEXT:             OpReturn
// CHECK-NEXT:             OpFunctionEnd
void main() {
    static RWTexture2D<float4> lRWTex;
}
