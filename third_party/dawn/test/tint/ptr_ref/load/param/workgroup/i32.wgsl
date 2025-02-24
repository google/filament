var<workgroup> S : i32;

fn func(pointer : ptr<workgroup, i32>) -> i32 {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S);
}
