fn Bad(index : u32, rd : vec3<f32>) -> vec3<f32> {
  var normal : vec3<f32> = vec3<f32>(0.0);
  normal[index] = -(sign(rd[index]));
  return normalize(normal);
}

struct S {
  v : vec3<f32>,
  i : u32,
}

@binding(0) @group(0) var<storage, read_write> io : S;

@compute @workgroup_size(1)
fn main(@builtin(local_invocation_index) idx : u32) {
  io.v = Bad(io.i, io.v);
}
