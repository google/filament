struct T {
  k : array<u32, 2>,
}

struct S {
  m : i32,
  n : array<T, 4>,
}

fn f() -> u32 {
  var a = S();
  return a.n[2].k[1];
}
