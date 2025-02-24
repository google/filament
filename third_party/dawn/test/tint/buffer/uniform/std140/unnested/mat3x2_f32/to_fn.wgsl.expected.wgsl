@group(0) @binding(0) var<uniform> u : mat3x2<f32>;

fn a(m : mat3x2<f32>) {
}

fn b(v : vec2<f32>) {
}

fn c(f : f32) {
}

@compute @workgroup_size(1)
fn f() {
  a(u);
  b(u[1]);
  b(u[1].yx);
  c(u[1].x);
  c(u[1].yx.x);
}
