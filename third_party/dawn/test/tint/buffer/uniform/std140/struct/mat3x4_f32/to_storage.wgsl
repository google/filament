struct S {
  before : i32,
  m : mat3x4<f32>,
  @align(64) @size(16)
  after : i32,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

@compute @workgroup_size(1)
fn f() {
    s = u;
    s[1] = u[2];
    s[3].m = u[2].m;
    s[1].m[0] = u[0].m[1].ywxz;
}
