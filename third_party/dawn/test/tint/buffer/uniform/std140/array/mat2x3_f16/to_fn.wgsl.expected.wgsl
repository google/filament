enable f16;

@group(0) @binding(0) var<uniform> u : array<mat2x3<f16>, 4>;

@group(0) @binding(1) var<storage, read_write> s : f16;

fn a(a : array<mat2x3<f16>, 4>) -> f16 {
  return a[0][0].x;
}

fn b(m : mat2x3<f16>) -> f16 {
  return m[0].x;
}

fn c(v : vec3<f16>) -> f16 {
  return v.x;
}

fn d(f : f16) -> f16 {
  return f;
}

@compute @workgroup_size(1)
fn f() {
  s = (((a(u) + b(u[1])) + c(u[1][0].zxy)) + d(u[1][0].zxy.x));
}
