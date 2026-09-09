// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s  --implicit-check-not "OpDecorate {{%[0-9]+}} NoContraction"

// The --implicit-check-not option above checks that there are no `OpDecorate ... NoContraction` instructions other than those CHECKed below.

struct S {
            float4 a;
    precise float4 ap;
               int b[5];
       precise int bp[5];
            int2x3 c[6][7][8];
    precise int2x3 cp[6][7][8];
          float3x4 d;
  precise float3x4 dp;
};

struct T {
  precise S sub1; // all members of sub1 should be precise.
  S sub2; // only some members of sub2 are precise.
};


// CHECK:      OpName %w "w"
// CHECK-NEXT: OpDecorate [[x_mul_x_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[xx_plus_y_2:%[0-9]+]] NoContraction

// CHECK-NEXT: OpDecorate [[z2_mul_z3_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[z2z3_plus_z4_2:%[0-9]+]] NoContraction

// CHECK-NEXT: OpDecorate [[uu_row0_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[uu_row1_2:%[0-9]+]] NoContraction

// CHECK-NEXT: OpDecorate [[ww_row0_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row1_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row2_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row0_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row1_2:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row2_2:%[0-9]+]] NoContraction

// CHECK-NEXT: OpDecorate [[x_mul_x_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[xx_plus_y_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[x_mul_x_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[xx_plus_y_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[z2_mul_z3_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[z2z3_plus_z4_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[z2_mul_z3_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[z2z3_plus_z4_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[uu_row0_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[uu_row1_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[uu_row0_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[uu_row1_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row0_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row1_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row2_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row0_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row1_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row2_3:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row0_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row1_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_row2_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row0_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row1_4:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[ww_plus_w_row2_4:%[0-9]+]] NoContraction

void main() {
  T t;
  float4 x,y;
  int z[5];
  int2x3 u;
  float3x4 w;


// 'a' is NOT precise.
//
// CHECK:   [[x_mul_x_1:%[0-9]+]] = OpFMul %v4float
// CHECK: [[xx_plus_y_1:%[0-9]+]] = OpFAdd %v4float
  t.sub2.a = x * x + y;

// 'ap' is precise.
//
// CHECK:   [[x_mul_x_2]] = OpFMul %v4float
// CHECK: [[xx_plus_y_2]] = OpFAdd %v4float
  t.sub2.ap = x * x + y;

// 'b' is NOT precise.
//
// CHECK:    [[z2_mul_z3_1:%[0-9]+]] = OpIMul %int
// CHECK: [[z2z3_plus_z4_1:%[0-9]+]] = OpIAdd %int
  t.sub2.b[1] = z[2] * z[3] + z[4];

// 'bp' is precise.
//
// CHECK:    [[z2_mul_z3_2]] = OpIMul %int
// CHECK: [[z2z3_plus_z4_2]] = OpIAdd %int
  t.sub2.bp[1] = z[2] * z[3] + z[4];

// 'c' is NOT precise.
//
// CHECK: [[uu_row0_1:%[0-9]+]] = OpIMul %v3int
// CHECK: [[uu_row1_1:%[0-9]+]] = OpIMul %v3int
  t.sub2.c[0][1][2] = u * u;

// 'cp' is precise.
//
// CHECK: [[uu_row0_2]] = OpIMul %v3int
// CHECK: [[uu_row1_2]] = OpIMul %v3int
  t.sub2.cp[0][1][2] = u * u;

// 'd' is NOT precise.
//
// CHECK:        [[ww_row0_1:%[0-9]+]] = OpFMul %v4float
// CHECK:        [[ww_row1_1:%[0-9]+]] = OpFMul %v4float
// CHECK:        [[ww_row2_1:%[0-9]+]] = OpFMul %v4float
// CHECK: [[ww_plus_w_row0_1:%[0-9]+]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row1_1:%[0-9]+]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row2_1:%[0-9]+]] = OpFAdd %v4float
  t.sub2.d = w * w + w;

// 'dp' is precise.
//
// CHECK:        [[ww_row0_2]] = OpFMul %v4float
// CHECK:        [[ww_row1_2]] = OpFMul %v4float
// CHECK:        [[ww_row2_2]] = OpFMul %v4float
// CHECK: [[ww_plus_w_row0_2]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row1_2]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row2_2]] = OpFAdd %v4float
  t.sub2.dp = w * w + w;

// *ALL* members of sub1 are precise. So this operation should be precise.
//
//
// CHECK:   [[x_mul_x_3]] = OpFMul %v4float
// CHECK: [[xx_plus_y_3]] = OpFAdd %v4float
  t.sub1.a = x * x + y;
// CHECK:   [[x_mul_x_4]] = OpFMul %v4float
// CHECK: [[xx_plus_y_4]] = OpFAdd %v4float
  t.sub1.ap = x * x + y;
// CHECK:    [[z2_mul_z3_3]] = OpIMul %int
// CHECK: [[z2z3_plus_z4_3]] = OpIAdd %int
  t.sub1.b[1] = z[2] * z[3] + z[4];
// CHECK:    [[z2_mul_z3_4]] = OpIMul %int
// CHECK: [[z2z3_plus_z4_4]] = OpIAdd %int
  t.sub1.bp[1] = z[2] * z[3] + z[4];
// CHECK: [[uu_row0_3]] = OpIMul %v3int
// CHECK: [[uu_row1_3]] = OpIMul %v3int
  t.sub1.c[0][1][2] = u * u;
// CHECK: [[uu_row0_4]] = OpIMul %v3int
// CHECK: [[uu_row1_4]] = OpIMul %v3int
  t.sub1.cp[0][1][2] = u * u;
// CHECK:        [[ww_row0_3]] = OpFMul %v4float
// CHECK:        [[ww_row1_3]] = OpFMul %v4float
// CHECK:        [[ww_row2_3]] = OpFMul %v4float
// CHECK: [[ww_plus_w_row0_3]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row1_3]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row2_3]] = OpFAdd %v4float
  t.sub1.d = w * w + w;
// CHECK:        [[ww_row0_4]] = OpFMul %v4float
// CHECK:        [[ww_row1_4]] = OpFMul %v4float
// CHECK:        [[ww_row2_4]] = OpFMul %v4float
// CHECK: [[ww_plus_w_row0_4]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row1_4]] = OpFAdd %v4float
// CHECK: [[ww_plus_w_row2_4]] = OpFAdd %v4float
  t.sub1.dp = w * w + w;
}

