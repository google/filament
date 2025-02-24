@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

@fragment
fn main() {
  if (non_uniform_global < 0) {
    discard;
  }
  {
    {
      return;
    }
  }
}
