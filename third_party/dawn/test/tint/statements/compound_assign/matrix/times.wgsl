struct S {
  a : mat4x4<f32>,
}
@group(0) @binding(0)
var<storage, read_write> v : S;

fn foo() {
  v.a *= mat4x4<f32>();
}
