enable f16;

@group(0) @binding(0) var<uniform> u : array<mat2x3<f16>, 4>;

@group(0) @binding(1) var<storage, read_write> s : f16;

var<workgroup> w : array<mat2x3<f16>, 4>;

@compute @workgroup_size(1)
fn f() {
  w = u;
  w[1] = u[2];
  w[1][0] = u[0][1].zxy;
  w[1][0].x = u[0][1].x;
  s = w[1][0].x;
}
