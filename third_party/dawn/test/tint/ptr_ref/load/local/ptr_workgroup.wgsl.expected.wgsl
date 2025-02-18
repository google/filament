var<workgroup> i : i32;

@compute @workgroup_size(1)
fn main() {
  i = 123;
  let p : ptr<workgroup, i32> = &(i);
  let u : i32 = (*(p) + 1);
}
