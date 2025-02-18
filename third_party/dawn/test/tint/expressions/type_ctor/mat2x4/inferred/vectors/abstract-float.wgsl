const m = mat2x4(vec4(0.0, 1.0, 2.0, 3.0),
                 vec4(4.0, 5.0, 6.0, 7.0));

@group(0) @binding(0)
var<storage, read_write> out : mat2x4<f32>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
