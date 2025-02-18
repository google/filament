struct T {
  o: f32,
  p: u32
}

struct S {
  m: i32,
  n: T,
}

fn f() -> u32 {
  var a = S();
  return a.n.p;
}
