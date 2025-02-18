@group(0) @binding(0) var<storage, read_write> a : atomic<u32>;

@compute @workgroup_size(16)
fn main() {
  var value = 42u;
  let result = atomicCompareExchangeWeak(&(a), 0u, value);
}
