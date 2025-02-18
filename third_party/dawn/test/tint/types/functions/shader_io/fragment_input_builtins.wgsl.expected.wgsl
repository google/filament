@fragment
fn main(@builtin(position) position : vec4<f32>, @builtin(front_facing) front_facing : bool, @builtin(sample_index) sample_index : u32, @builtin(sample_mask) sample_mask : u32) {
  if (front_facing) {
    let foo : vec4<f32> = position;
    let bar : u32 = (sample_index + sample_mask);
  }
}
