// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float val;

    float getVal() { return val; }
};

static S gSVar = {4.2};

float main() : A {
// CHECK:      [[ret:%[0-9]+]] = OpFunctionCall %float %S_getVal %gSVar
// CHECK-NEXT:                OpReturnValue [[ret]]
    return gSVar.getVal();
}
