@group(0) @binding(0) var<uniform> S : i32;

fn func(pointer : ptr<uniform, i32>) -> i32 {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S));
}
