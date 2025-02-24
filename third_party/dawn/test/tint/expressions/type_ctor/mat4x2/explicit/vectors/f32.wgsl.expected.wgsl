var<private> m = mat4x2<f32>(vec2<f32>(0.0f, 1.0f), vec2<f32>(2.0f, 3.0f), vec2<f32>(4.0f, 5.0f), vec2<f32>(6.0f, 7.0f));

@group(0) @binding(0) var<storage, read_write> out : mat4x2<f32>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
