struct S {
  x : i32,
  a : atomic<u32>,
  b : atomic<u32>,
};

var<workgroup> wg: S;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore(&wg.a, 1u);
  atomicStore(&wg.b, 2u);
}
