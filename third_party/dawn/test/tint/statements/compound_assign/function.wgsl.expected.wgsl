fn foo() {
  var<function> a : i32;
  var<function> b : vec4<f32>;
  var<function> c : mat2x2<f32>;
  a /= 2;
  b *= mat4x4<f32>();
  c *= 2.0;
}
