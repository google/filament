@group(0) @binding(0) var<uniform> u : array<mat4x2<f32>, 4>;

@group(0) @binding(1) var<storage, read_write> s : f32;

fn a(a : array<mat4x2<f32>, 4>) -> f32 {
  return a[0][0].x;
}

fn b(m : mat4x2<f32>) -> f32 {
  return m[0].x;
}

fn c(v : vec2<f32>) -> f32 {
  return v.x;
}

fn d(f : f32) -> f32 {
  return f;
}

@compute @workgroup_size(1)
fn f() {
  s = (((a(u) + b(u[1])) + c(u[1][0].yx)) + d(u[1][0].yx.x));
}
