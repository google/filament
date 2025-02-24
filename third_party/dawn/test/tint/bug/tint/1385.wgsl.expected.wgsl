@group(0) @binding(1) var<storage, read> data : array<i32>;

fn foo() -> i32 {
  return data[0];
}

@compute @workgroup_size(16, 16, 1)
fn main() {
  foo();
}
