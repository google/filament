
@group(0) @binding(0) var<storage, read_write> S : atomic<i32>;

@fragment
fn main() -> @location(0) vec4f {
  if (false) {
    discard;
  }
  let old_value = atomicCompareExchangeWeak(&S, 0i, 1i).old_value;
  return vec4f(f32(old_value));
}
