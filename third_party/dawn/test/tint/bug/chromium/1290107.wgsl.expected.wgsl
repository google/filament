struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<S>;

@compute @workgroup_size(1)
fn main() {
  let len = arrayLength(&(arr));
}
