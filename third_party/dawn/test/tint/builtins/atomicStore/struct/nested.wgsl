struct S0 {
  x : i32,
  a : atomic<u32>,
  y : i32,
  z : i32,
};

struct S1 {
  x : i32,
  a : S0,
  y : i32,
  z : i32,
};

struct S2 {
  x : i32,
  y : i32,
  z : i32,
  a : S1,
};

var<workgroup> wg: S2;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore(&wg.a.a.a, 1u);
}
