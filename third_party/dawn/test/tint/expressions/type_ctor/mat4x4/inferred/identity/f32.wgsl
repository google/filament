var<private> m = mat4x4(mat4x4(0.0f, 1.0f, 2.0f, 3.0f,
                               4.0f, 5.0f, 6.0f, 7.0f,
                               8.0f, 9.0f, 10.0f, 11.0f,
                               12.0f, 13.0f, 14.0f, 15.0f));

@group(0) @binding(0)
var<storage, read_write> out : mat4x4<f32>;

@compute @workgroup_size(1)
fn f() {
  out = mat4x4(m);
}
