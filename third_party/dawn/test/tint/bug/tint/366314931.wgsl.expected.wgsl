struct S {
  v : vec3u,
  u : atomic<u32>,
}

var<workgroup> wgvar : S;

@group(0) @binding(0) var<storage, read_write> output : S;

@compute @workgroup_size(1, 1, 1)
fn main() {
  let x = atomicLoad(&(wgvar.u));
  atomicStore(&(output.u), x);
}
