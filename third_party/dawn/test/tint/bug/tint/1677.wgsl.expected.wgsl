struct Input {
  position : vec3<i32>,
}

@group(0) @binding(0) var<storage, read> input : Input;

@compute @workgroup_size(1, 1, 1)
fn main(@builtin(global_invocation_id) id : vec3<u32>) {
  let pos = (input.position - vec3<i32>(0));
}
