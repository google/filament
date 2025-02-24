var<private> my_global : vec4<f32>;

@group(0) @binding(0) var<uniform> my_uniform : f32;

@group(0) @binding(1) var my_texture : texture_2d<f32>;

@group(0) @binding(2) var my_sampler : sampler;

fn foo_member_initialize() {
  var vb2 : vec2<bool>;
  vb2.x = (my_global.z != 0);
  vb2.x = (my_uniform == -(1.0f));
  vb2 = vec2((my_uniform == -(1.0f)), false);
  if (vb2.x) {
    let r : vec4<f32> = textureSampleBias(my_texture, my_sampler, vec2<f32>(), 0.0);
  }
}

fn foo_default_initialize() {
  var vb2 : vec2<bool>;
  vb2.x = (my_global.z != 0);
  vb2.x = (my_uniform == -(1.0f));
  vb2 = vec2<bool>();
  if (vb2.x) {
    let r : vec4<f32> = textureSampleBias(my_texture, my_sampler, vec2<f32>(), 0.0);
  }
}
