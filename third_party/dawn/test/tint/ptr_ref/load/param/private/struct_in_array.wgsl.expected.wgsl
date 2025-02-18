struct str {
  i : i32,
}

fn func(pointer : ptr<private, str>) -> str {
  return *(pointer);
}

var<private> P : array<str, 4>;

@compute @workgroup_size(1)
fn main() {
  let r = func(&(P[2]));
}
