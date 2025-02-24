const foo = (false && (array<f32, 4>()[0] == 0));

const bar = (false && (mat4x4<f32>()[0][0] == 0));

@group(0) @binding(0) var<storage, read_write> output : array<u32, 2>;

@compute @workgroup_size(1)
fn main() {
  if (foo) {
    output[0] = 1;
  }
  if (bar) {
    output[1] = 1;
  }
}
