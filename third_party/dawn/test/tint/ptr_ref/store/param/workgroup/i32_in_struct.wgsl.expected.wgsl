struct str {
  i : i32,
}

var<workgroup> S : str;

fn func(pointer : ptr<workgroup, i32>) {
  *(pointer) = 42;
}

@compute @workgroup_size(1)
fn main() {
  func(&(S.i));
}
