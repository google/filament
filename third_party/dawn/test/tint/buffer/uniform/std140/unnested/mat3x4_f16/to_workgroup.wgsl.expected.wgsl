enable f16;

@group(0) @binding(0) var<uniform> u : mat3x4<f16>;

var<workgroup> w : mat3x4<f16>;

@compute @workgroup_size(1)
fn f() {
  w = u;
  w[1] = u[0];
  w[1] = u[0].ywxz;
  w[0][1] = u[1][0];
}
