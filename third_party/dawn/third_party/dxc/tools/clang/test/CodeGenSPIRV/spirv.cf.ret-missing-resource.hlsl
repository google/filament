// RUN: %dxc -T vs_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: [[undef:%[0-9]+]] = OpUndef %type_2d_image

Texture2D texA;
Texture2D texB;

// CHECK:      %select = OpFunction
Texture2D select(bool lhs) {

// CHECK:      %if_true = OpLabel
  if (lhs)
    return texA;
// CHECK:      %if_false = OpLabel
  else
    return texB;

  // no return for dead branch.
// CHECK:      %if_merge = OpLabel
// CHECK-NEXT: OpReturnValue [[undef]]
}
// CHECK-NEXT: OpFunctionEnd

float main(bool a: A) : B {
    Texture2D tex = select(true);
    return 1.0;
}
