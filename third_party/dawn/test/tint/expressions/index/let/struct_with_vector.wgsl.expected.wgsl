struct S {
  m : i32,
  n : vec3u,
}

fn f() -> u32 {
  let a = S();
  return a.n[2];
}
