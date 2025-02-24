@group(0) @binding(0) var arg_0 : texture_1d<i32>;

fn d() {
  _ = textureLoad(arg_0, 1, 0);
  let l = sin(3.0);
}
