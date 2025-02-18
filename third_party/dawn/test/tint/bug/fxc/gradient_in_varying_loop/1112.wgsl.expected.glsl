#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D randomTexture_Sampler;
layout(location = 0) in vec2 tint_interstage_location0;
layout(location = 0) out vec4 main_loc0_Output;
vec4 main_inner(vec2 vUV) {
  vec3 random = texture(randomTexture_Sampler, vUV).xyz;
  int i = 0;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((i < 1)) {
      } else {
        break;
      }
      vec3 offset = vec3(random.x);
      bool v = false;
      if ((offset.x < 0.0f)) {
        v = true;
      } else {
        v = (offset.y < 0.0f);
      }
      bool v_1 = false;
      if (v) {
        v_1 = true;
      } else {
        v_1 = (offset.x > 1.0f);
      }
      bool v_2 = false;
      if (v_1) {
        v_2 = true;
      } else {
        v_2 = (offset.y > 1.0f);
      }
      if (v_2) {
        i = (i + 1);
        {
          uint tint_low_inc = (tint_loop_idx.x + 1u);
          tint_loop_idx.x = tint_low_inc;
          uint tint_carry = uint((tint_low_inc == 0u));
          tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        }
        continue;
      }
      float sampleDepth = 0.0f;
      i = (i + 1);
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
      }
      continue;
    }
  }
  return vec4(1.0f);
}
void main() {
  main_loc0_Output = main_inner(tint_interstage_location0);
}
