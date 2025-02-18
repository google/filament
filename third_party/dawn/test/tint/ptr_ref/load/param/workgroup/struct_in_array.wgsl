struct str {
  i : i32,
};

var<workgroup> S : array<str, 4>;

fn func(pointer : ptr<workgroup, str>) -> str {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S[2]);
}
