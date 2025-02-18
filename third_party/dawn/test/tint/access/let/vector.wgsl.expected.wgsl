@group(0) @binding(0) var<storage, read_write> s : vec3f;

@compute @workgroup_size(1)
fn main() {
  let v : vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
  let scalar : f32 = v.y;
  let swizzle2 : vec2<f32> = v.xz;
  let swizzle3 : vec3<f32> = v.xzy;
  s = ((vec3(scalar) + vec3(swizzle2, 1)) + swizzle3);
}
