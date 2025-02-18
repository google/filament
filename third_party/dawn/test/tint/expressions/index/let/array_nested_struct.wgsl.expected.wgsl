struct S {
  m : i32,
  n : array<u32, 4>,
}

fn f() -> u32 {
  let a : array<S, 2> = array<S, 2>();
  return a[1].n[1];
}
