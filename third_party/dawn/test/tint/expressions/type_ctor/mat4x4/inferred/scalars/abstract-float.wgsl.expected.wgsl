const m = mat4x4(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);

@group(0) @binding(0) var<storage, read_write> out : mat4x4<f32>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
