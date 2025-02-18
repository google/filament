@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16uint, read_write>;

fn textureStore_9ba5c1() {
  var arg_1 = 1u;
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_9ba5c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9ba5c1();
}
