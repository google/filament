struct S {
  x : i32,
  a : atomic<u32>,
  y : u32,
}

var<workgroup> wg : array<S, 10>;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore(&(wg[4].a), 1u);
}
