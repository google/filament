@group(0) @binding(0) var<storage, read_write> s: i32;

fn f(_a : i32) {
  let b = _a;

  s = b;
}

@compute @workgroup_size(1)
fn main() {
  f(1);
}
