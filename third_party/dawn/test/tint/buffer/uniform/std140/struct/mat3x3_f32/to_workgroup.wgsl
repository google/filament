struct S {
  before : i32,
  m : mat3x3<f32>,
  @align(64) @size(16)
  after : i32,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
var<workgroup> w : array<S, 4>;

@compute @workgroup_size(1)
fn f() {
    w = u;
    w[1] = u[2];
    w[3].m = u[2].m;
    w[1].m[0] = u[0].m[1].zxy;
}
