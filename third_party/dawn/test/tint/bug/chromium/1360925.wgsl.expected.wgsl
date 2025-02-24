@group(0) @binding(0) var<storage> G : array<i32>;

fn n() {
  let p = &(G);
  _ = arrayLength(p);
}
