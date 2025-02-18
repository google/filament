struct str {
  i : i32,
}

var<workgroup> S : array<str, 4>;

fn func(pointer : ptr<workgroup, str>) {
  *(pointer) = str();
}

@compute @workgroup_size(1)
fn main() {
  func(&(S[2]));
}
