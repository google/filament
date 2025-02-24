struct FragmentInputs {
  @builtin(position)
  position : vec4<f32>,
  @builtin(front_facing)
  front_facing : bool,
  @builtin(sample_index)
  sample_index : u32,
  @builtin(sample_mask)
  sample_mask : u32,
}

@fragment
fn main(inputs : FragmentInputs) {
  if (inputs.front_facing) {
    let foo : vec4<f32> = inputs.position;
    let bar : u32 = (inputs.sample_index + inputs.sample_mask);
  }
}
