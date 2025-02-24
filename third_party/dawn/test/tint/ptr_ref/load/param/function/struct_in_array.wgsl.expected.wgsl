struct str {
  i : i32,
}

fn func(pointer : ptr<function, str>) -> str {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  var F : array<str, 4>;
  let r = func(&(F[2]));
}
