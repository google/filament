struct S {
  m : i32,
  n : u32,
}

fn f() -> u32 {
  var a = S();
  return a.n;
}
