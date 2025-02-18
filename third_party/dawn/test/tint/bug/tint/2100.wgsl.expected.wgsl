struct S {
  matrix_view : mat4x4<f32>,
  matrix_normal : mat3x3<f32>,
}

@group(0) @binding(0) var<uniform> buffer : S;

@vertex
fn main() -> @builtin(position) vec4f {
  let x = buffer.matrix_view[0].z;
  return vec4f(x, 0, 0, 1);
}
