@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8sint, read_write>;

fn textureStore_28e109() {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  textureStore_28e109();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_28e109();
}
