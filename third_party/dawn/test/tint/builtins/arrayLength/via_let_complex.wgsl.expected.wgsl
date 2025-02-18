struct S {
  a : array<i32>,
}

@group(0) @binding(0) var<storage, read> G : S;

@compute @workgroup_size(1)
fn main() {
  let p = &(*(&(G)));
  let p2 = &(*(p));
  let p3 = &((*(p)).a);
  let l1 : u32 = arrayLength(&(*(p3)));
}
