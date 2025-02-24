@group(0) @binding(0) var<storage, read_write> s: i32;

fn f(_A : i32) {
  let B = _A;

  s = B;
}

@compute @workgroup_size(1)
fn main() {
  f(1);
}
