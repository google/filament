var<private> m = mat3x4(vec4<f32>(0.0f, 1.0f, 2.0f, 3.0f), vec4<f32>(4.0f, 5.0f, 6.0f, 7.0f), vec4<f32>(8.0f, 9.0f, 10.0f, 11.0f));

@group(0) @binding(0) var<storage, read_write> out : mat3x4<f32>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
