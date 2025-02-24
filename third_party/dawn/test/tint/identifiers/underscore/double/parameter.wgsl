@group(0) @binding(0) var<storage, read_write> s: i32;

fn f(a__ : i32) {
  let b = a__;

  s = b;
}

@compute @workgroup_size(1)
fn main() {
  f(1);
}
