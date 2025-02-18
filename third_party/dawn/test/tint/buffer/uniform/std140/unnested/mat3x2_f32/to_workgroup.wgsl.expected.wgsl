@group(0) @binding(0) var<uniform> u : mat3x2<f32>;

var<workgroup> w : mat3x2<f32>;

@compute @workgroup_size(1)
fn f() {
  w = u;
  w[1] = u[0];
  w[1] = u[0].yx;
  w[0][1] = u[1][0];
}
