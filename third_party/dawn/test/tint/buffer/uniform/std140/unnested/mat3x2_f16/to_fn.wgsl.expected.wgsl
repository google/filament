enable f16;

@group(0) @binding(0) var<uniform> u : mat3x2<f16>;

fn a(m : mat3x2<f16>) {
}

fn b(v : vec2<f16>) {
}

fn c(f : f16) {
}

@compute @workgroup_size(1)
fn f() {
  a(u);
  b(u[1]);
  b(u[1].yx);
  c(u[1].x);
  c(u[1].yx.x);
}
