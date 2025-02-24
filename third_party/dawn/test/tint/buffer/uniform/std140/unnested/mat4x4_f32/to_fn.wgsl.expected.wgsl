@group(0) @binding(0) var<uniform> u : mat4x4<f32>;

fn a(m : mat4x4<f32>) {
}

fn b(v : vec4<f32>) {
}

fn c(f : f32) {
}

@compute @workgroup_size(1)
fn f() {
  a(u);
  b(u[1]);
  b(u[1].ywxz);
  c(u[1].x);
  c(u[1].ywxz.x);
}
