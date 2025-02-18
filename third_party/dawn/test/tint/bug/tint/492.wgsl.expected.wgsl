struct S {
  a : i32,
}

@group(0) @binding(0) var<storage, read_write> buf : S;

@compute @workgroup_size(1)
fn main() {
  let p : ptr<storage, i32, read_write> = &(buf.a);
  *(p) = 12;
}
