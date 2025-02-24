@group(0) @binding(0)
var<storage, read_write> a_u32 : atomic<u32>;

@group(0) @binding(1)
var<storage, read_write> a_i32 : atomic<i32>;

var<workgroup> b_u32 : atomic<u32>;

var<workgroup> b_i32 : atomic<i32>;


@compute @workgroup_size(16)
fn main() {
  {
    var value = 42u;
    let r1 = atomicCompareExchangeWeak(&a_u32, 0u, value);
    let r2 = atomicCompareExchangeWeak(&a_u32, 0u, value);
    let r3 = atomicCompareExchangeWeak(&a_u32, 0u, value);
  }
  {
    var value = 42;
    let r1 = atomicCompareExchangeWeak(&a_i32, 0, value);
    let r2 = atomicCompareExchangeWeak(&a_i32, 0, value);
    let r3 = atomicCompareExchangeWeak(&a_i32, 0, value);
  }
  {
    var value = 42u;
    let r1 = atomicCompareExchangeWeak(&b_u32, 0u, value);
    let r2 = atomicCompareExchangeWeak(&b_u32, 0u, value);
    let r3 = atomicCompareExchangeWeak(&b_u32, 0u, value);
  }
  {
    var value = 42;
    let r1 = atomicCompareExchangeWeak(&b_i32, 0, value);
    let r2 = atomicCompareExchangeWeak(&b_i32, 0, value);
    let r3 = atomicCompareExchangeWeak(&b_i32, 0, value);
  }

}
