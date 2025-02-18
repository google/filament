enable f16;

@group(0) @binding(0) var<uniform> u : mat2x3<f16>;

fn a(m : mat2x3<f16>) {
}

fn b(v : vec3<f16>) {
}

fn c(f : f16) {
}

@compute @workgroup_size(1)
fn f() {
  a(u);
  b(u[1]);
  b(u[1].zxy);
  c(u[1].x);
  c(u[1].zxy.x);
}
