struct str {
  i : i32,
}

@group(0) @binding(0) var<storage> S : str;

fn func(pointer : ptr<storage, i32>) -> i32 {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S.i));
}
