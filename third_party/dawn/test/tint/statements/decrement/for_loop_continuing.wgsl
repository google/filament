@group(0) @binding(0) var<storage, read_write> i : u32;

fn main() {
  for (; i < 10u; i--) {
  }
}
