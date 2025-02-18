struct str {
  i : i32,
}

@group(0) @binding(0) var<storage> S : array<str, 4>;

fn func(pointer : ptr<storage, str>) -> str {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S[2]));
}
