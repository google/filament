struct str {
  i : i32,
}

fn func(pointer : ptr<function, str>) {
  *(pointer) = str();
}

@compute @workgroup_size(1)
fn main() {
  var F : array<str, 4>;
  func(&(F[2]));
}
