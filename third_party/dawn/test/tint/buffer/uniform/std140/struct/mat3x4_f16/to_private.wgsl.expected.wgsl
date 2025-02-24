enable f16;

struct S {
  before : i32,
  m : mat3x4<f16>,
  @align(64) @size(16)
  after : i32,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

var<private> p : array<S, 4>;

@compute @workgroup_size(1)
fn f() {
  p = u;
  p[1] = u[2];
  p[3].m = u[2].m;
  p[1].m[0] = u[0].m[1].ywxz;
}
