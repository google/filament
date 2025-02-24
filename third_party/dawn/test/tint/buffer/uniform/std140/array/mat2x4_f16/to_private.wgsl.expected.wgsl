enable f16;

@group(0) @binding(0) var<uniform> u : array<mat2x4<f16>, 4>;

@group(0) @binding(1) var<storage, read_write> s : f16;

var<private> p : array<mat2x4<f16>, 4>;

@compute @workgroup_size(1)
fn f() {
  p = u;
  p[1] = u[2];
  p[1][0] = u[0][1].ywxz;
  p[1][0].x = u[0][1].x;
  s = p[1][0].x;
}
