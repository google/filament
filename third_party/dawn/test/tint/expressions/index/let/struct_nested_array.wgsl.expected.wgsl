struct S {
  m : i32,
  n : array<u32, 4>,
}

fn f() -> u32 {
  let a = S();
  return a.n[2];
}
