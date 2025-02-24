@group(0) @binding(0) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo() -> @location(0) i32 {
  discard;
  var x = 0;
  let result = atomicCompareExchangeWeak(&a, 0, 1);
  if (result.exchanged) {
    x = result.old_value;
  }
  return x;
}
