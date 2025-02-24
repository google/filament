var<private> m = mat2x2<f32>();

@group(0) @binding(0)
var<storage, read_write> out : mat2x2<f32>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
