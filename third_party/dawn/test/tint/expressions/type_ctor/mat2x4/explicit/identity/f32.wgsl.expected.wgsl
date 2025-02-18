var<private> m = mat2x4<f32>(mat2x4<f32>(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f));

@group(0) @binding(0) var<storage, read_write> out : mat2x4<f32>;

@compute @workgroup_size(1)
fn f() {
  out = mat2x4<f32>(m);
}
