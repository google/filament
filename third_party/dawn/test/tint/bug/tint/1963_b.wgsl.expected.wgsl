@group(0) @binding(0) var<storage, read_write> a : atomic<i32>;

@compute @workgroup_size(1)
fn compute_main() {
  let v = atomicCompareExchangeWeak(&(a), 1, 1).old_value;
}
