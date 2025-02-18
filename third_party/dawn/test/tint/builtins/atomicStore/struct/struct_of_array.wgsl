struct S {
  x : i32,
  a : array<atomic<u32>, 10>,
  y : u32,
};

var<workgroup> wg : S;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore(&wg.a[4], 1u);
}
