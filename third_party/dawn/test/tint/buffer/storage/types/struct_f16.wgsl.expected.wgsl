enable f16;

struct Inner {
  scalar_f16 : f16,
  vec3_f16 : vec3<f16>,
  mat2x4_f16 : mat2x4<f16>,
}

struct S {
  inner : Inner,
}

@group(0) @binding(0) var<storage, read> in : S;

@group(0) @binding(1) var<storage, read_write> out : S;

@compute @workgroup_size(1)
fn main() {
  let t = in;
  out = t;
}
