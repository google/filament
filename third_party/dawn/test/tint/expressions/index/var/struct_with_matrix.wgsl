struct S {
  m: i32,
  n: mat4x4f,
}

fn f() -> f32 {
  var a = S();
  return a.n[2][1];
}
