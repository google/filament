enable f16;

@group(0) @binding(0) var<uniform> u : mat2x2<f16>;

var<workgroup> w : mat2x2<f16>;

@compute @workgroup_size(1)
fn f() {
  w = u;
  w[1] = u[0];
  w[1] = u[0].yx;
  w[0][1] = u[1][0];
}
