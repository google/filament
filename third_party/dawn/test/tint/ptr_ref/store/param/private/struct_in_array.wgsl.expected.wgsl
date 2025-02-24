struct str {
  i : i32,
}

fn func(pointer : ptr<private, str>) {
  *(pointer) = str();
}

var<private> P : array<str, 4>;

@compute @workgroup_size(1)
fn main() {
  func(&(P[2]));
}
