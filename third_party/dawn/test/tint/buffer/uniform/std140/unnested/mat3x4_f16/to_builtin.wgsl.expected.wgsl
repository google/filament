enable f16;

@group(0) @binding(0) var<uniform> u : mat3x4<f16>;

@compute @workgroup_size(1)
fn f() {
  let t = transpose(u);
  let l = length(u[1]);
  let a = abs(u[0].ywxz.x);
}
