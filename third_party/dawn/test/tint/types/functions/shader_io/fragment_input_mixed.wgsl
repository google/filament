struct FragmentInputs0 {
  @builtin(position) position : vec4<f32>,
  @location(0) @interpolate(flat) loc0 : i32,
};
struct FragmentInputs1 {
  @location(3) loc3 : vec4<f32>,
  @builtin(sample_mask) sample_mask : u32,
};

@fragment
fn main(
  inputs0 : FragmentInputs0,
  @builtin(front_facing) front_facing : bool,
  @location(1) @interpolate(flat) loc1 : u32,
  @builtin(sample_index) sample_index : u32,
  inputs1 : FragmentInputs1,
  @location(2) loc2 : f32,
) {
  if (front_facing) {
    let foo : vec4<f32> = inputs0.position;
    let bar : u32 = sample_index + inputs1.sample_mask;
    let i : i32 = inputs0.loc0;
    let u : u32 = loc1;
    let f : f32 = loc2;
    let v : vec4<f32> = inputs1.loc3;
  }
}
