// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v3float_1_1_1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1

RWTexture2D<float>  MyTexture : register(u1);
RWBuffer<int> intbuf;

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int a, b;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[a1:%[0-9]+]] = OpISub %int [[a0]] %int_1
// CHECK-NEXT: OpStore %a [[a1]]
// CHECK-NEXT: OpStore %b [[a0]]
    b = a--;

    uint i, j;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[i1:%[0-9]+]] = OpISub %uint [[i0]] %uint_1
// CHECK-NEXT: OpStore %i [[i1]]
// CHECK-NEXT: OpStore %j [[i0]]
    j = i--;

    float o, p;
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[o1:%[0-9]+]] = OpFSub %float [[o0]] %float_1
// CHECK-NEXT: OpStore %o [[o1]]
// CHECK-NEXT: OpStore %p [[o0]]
    p = o--;

    float3 x, y;
// CHECK-NEXT: [[x0:%[0-9]+]] = OpLoad %v3float %x
// CHECK-NEXT: [[x1:%[0-9]+]] = OpFSub %v3float [[x0]] [[v3float_1_1_1]]
// CHECK-NEXT: OpStore %x [[x1]]
// CHECK-NEXT: OpStore %y [[x0]]
    y = x--;

  uint2 index;
// CHECK:      [[index:%[0-9]+]] = OpLoad %v2uint %index
// CHECK-NEXT:   [[img:%[0-9]+]] = OpLoad %type_2d_image %MyTexture
// CHECK-NEXT:   [[vec:%[0-9]+]] = OpImageRead %v4float [[img]] [[index]] None
// CHECK-NEXT:   [[val:%[0-9]+]] = OpCompositeExtract %float [[vec]] 0
// CHECK-NEXT:   [[dec:%[0-9]+]] = OpFSub %float [[val]] %float_1
// CHECK:      [[index_0:%[0-9]+]] = OpLoad %v2uint %index
// CHECK-NEXT:   [[img_0:%[0-9]+]] = OpLoad %type_2d_image %MyTexture
// CHECK-NEXT:                  OpImageWrite [[img_0]] [[index_0]] [[dec]]
// CHECK-NEXT:                  OpStore %s [[val]]
  float s = MyTexture[index]--;

// CHECK:      [[img_1:%[0-9]+]] = OpLoad %type_buffer_image %intbuf
// CHECK-NEXT: [[vec_0:%[0-9]+]] = OpImageRead %v4int [[img_1]] %uint_1 None
// CHECK-NEXT: [[val_0:%[0-9]+]] = OpCompositeExtract %int [[vec_0]] 0
// CHECK-NEXT: [[dec_0:%[0-9]+]] = OpISub %int [[val_0]] %int_1
// CHECK-NEXT: [[img_2:%[0-9]+]] = OpLoad %type_buffer_image %intbuf
// CHECK-NEXT:       OpImageWrite [[img_2]] %uint_1 [[dec_0]]
// CHECK-NEXT:       OpStore %t [[val_0]]
  int t = intbuf[1]--;
}
