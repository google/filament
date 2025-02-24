// RUN: %dxc -T vs_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK:     [[undef:%[0-9]+]] = OpUndef %type_2d_image
// CHECK-NOT:                     OpUndef %type_2d_image


Texture2D texA;
Texture2D texB;

Texture2D select1(bool lhs) {
  if (lhs)
    return texA;
  else
    return texB;
  // no return for dead branch.
}

Texture2D select2(bool lhs) {
  if (lhs)
    return texA;
  else
    return texB;
  // no return for dead branch.
}

float main(bool a: A) : B {
    Texture2D x = select1(true);
    Texture2D y = select2(true);
    return 1.0;
}

