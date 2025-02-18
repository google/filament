enable f16;

struct S {
  before : i32,
  m : mat3x4<f16>,
  @align(64) @size(16)
  after : i32,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

fn a(a : array<S, 4>) {
}

fn b(s : S) {
}

fn c(m : mat3x4<f16>) {
}

fn d(v : vec4<f16>) {
}

fn e(f : f16) {
}

@compute @workgroup_size(1)
fn f() {
  a(u);
  b(u[2]);
  c(u[2].m);
  d(u[0].m[1].ywxz);
  e(u[0].m[1].ywxz.x);
}
