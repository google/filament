enable f16;

@group(0) @binding(0) var<uniform> u : array<mat2x4<f16>, 4>;

var<workgroup> w : array<mat2x4<f16>, 4>;

@compute @workgroup_size(1)
fn f() {
  w = u;
  w[1] = u[2];
  w[1][0] = u[0][1].ywxz;
  w[1][0].x = u[0][1].x;
}
