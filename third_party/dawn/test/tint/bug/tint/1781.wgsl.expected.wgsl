struct S {
  a : i32,
  b : f32,
}

fn f() {
  const v = (S(1, 2.0).a == 0);
}
