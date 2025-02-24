var<private> m = mat2x4<f32>(vec4<f32>(0.0f, 1.0f, 2.0f, 3.0f),
                             vec4<f32>(4.0f, 5.0f, 6.0f, 7.0f));

@group(0) @binding(0)
var<storage, read_write> out : mat2x4<f32>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
