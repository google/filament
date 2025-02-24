@fragment
fn main(
  @location(0) none : f32,
  @location(1) @interpolate(flat) flat : f32,
  @location(2) @interpolate(perspective, center) perspective_center : f32,
  @location(3) @interpolate(perspective, centroid) perspective_centroid : f32,
  @location(4) @interpolate(perspective, sample) perspective_sample : f32,
  @location(5) @interpolate(linear, center) linear_center : f32,
  @location(6) @interpolate(linear, centroid) linear_centroid : f32,
  @location(7) @interpolate(linear, sample) linear_sample : f32,
  @location(8) @interpolate(perspective) perspective_default : f32,
  @location(9) @interpolate(linear) linear_default : f32) {
}
