#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_non_uniform_global_block_ssbo {
  int inner;
} v;
layout(binding = 1, std430)
buffer f_output_block_ssbo {
  float inner;
} v_1;
bool continue_execution = true;
void main() {
  if ((v.inner < 0)) {
    continue_execution = false;
  }
  float v_2 = dFdx(1.0f);
  if (continue_execution) {
    v_1.inner = v_2;
  }
  if ((v_1.inner < 0.0f)) {
    int i = 0;
    {
      uvec2 tint_loop_idx = uvec2(0u);
      while(true) {
        if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
          break;
        }
        float v_3 = v_1.inner;
        if ((v_3 > float(i))) {
          float v_4 = float(i);
          if (continue_execution) {
            v_1.inner = v_4;
          }
          if (!(continue_execution)) {
            discard;
          }
          return;
        }
        {
          uint tint_low_inc = (tint_loop_idx.x + 1u);
          tint_loop_idx.x = tint_low_inc;
          uint tint_carry = uint((tint_low_inc == 0u));
          tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
          i = (i + 1);
          if ((i == 5)) { break; }
        }
        continue;
      }
    }
    if (!(continue_execution)) {
      discard;
    }
    return;
  }
  if (!(continue_execution)) {
    discard;
  }
}
