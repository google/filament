@binding(0) @group(0) var texture : texture_2d<f32>;

@compute @workgroup_size(6)
fn e() {
  {
    for(var level = textureNumLevels(texture); (level > 0); ) {
    }
  }
}
