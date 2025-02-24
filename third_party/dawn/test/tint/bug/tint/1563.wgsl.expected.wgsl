fn foo() -> f32 {
  let oob = 99;
  let b = vec4<f32>()[oob];
  var v : vec4<f32>;
  v[oob] = b;
  return b;
}
