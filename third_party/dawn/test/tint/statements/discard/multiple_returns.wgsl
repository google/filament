@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

@group(0) @binding(1) var<storage, read_write> output : f32;

@fragment
fn main() {
  if (non_uniform_global < 0) {
    discard;
  }
  output = dpdx(1.0);
  if (output < 0) {
    var i = 0;
    loop {
      if (output > f32(i)) {
        output = f32(i);
        return;
      }
      continuing {
        i++;
        break if i == 5;
      }
    }
    return;
  }
  return;
}
