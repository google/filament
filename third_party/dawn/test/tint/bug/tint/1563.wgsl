fn foo() -> f32 {
  let oob = 99;
  let b = vec4<f32>()[oob];  // 99 is out of bounds
  var v: vec4<f32>;
  v[oob] = b;  // 99 is out of bounds
  return b;
}
