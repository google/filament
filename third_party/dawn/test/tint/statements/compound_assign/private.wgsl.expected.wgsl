var<private> a : i32;

var<private> b : vec4<f32>;

var<private> c : mat2x2<f32>;

fn foo() {
  a /= 2;
  b *= mat4x4<f32>();
  c *= 2.0;
}
