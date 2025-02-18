struct S {
  a : i32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn main() {
  let p : ptr<storage, i32, read_write> = &(v.a);
  let u : i32 = (*(p) + 1);
}
