struct S {
  x : i32,
  a : atomic<u32>,
  y : u32,
};

var<workgroup> wg : S;

@compute @workgroup_size(1)
fn compute_main() {
  let p0 = &wg;
  let p1 = &((*p0).a);
  atomicStore(p1, 1u);
}
