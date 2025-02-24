//
// vs_main
//
#version 310 es


struct Uniforms {
  vec2 u_scale;
  vec2 u_offset;
};

struct VertexOutputs {
  vec2 texcoords;
  vec4 position;
};

layout(binding = 0, std140)
uniform v_uniforms_block_ubo {
  Uniforms inner;
} v;
layout(location = 0) out vec2 tint_interstage_location0;
VertexOutputs vs_main_inner(uint VertexIndex) {
  vec2 texcoord[3] = vec2[3](vec2(-0.5f, 0.0f), vec2(1.5f, 0.0f), vec2(0.5f, 2.0f));
  VertexOutputs v_1 = VertexOutputs(vec2(0.0f), vec4(0.0f));
  v_1.position = vec4(((texcoord[min(VertexIndex, 2u)] * 2.0f) - vec2(1.0f)), 0.0f, 1.0f);
  bool flipY = (v.inner.u_scale.y < 0.0f);
  if (flipY) {
    v_1.texcoords = ((((texcoord[min(VertexIndex, 2u)] * v.inner.u_scale) + v.inner.u_offset) * vec2(1.0f, -1.0f)) + vec2(0.0f, 1.0f));
  } else {
    v_1.texcoords = ((((texcoord[min(VertexIndex, 2u)] * vec2(1.0f, -1.0f)) + vec2(0.0f, 1.0f)) * v.inner.u_scale) + v.inner.u_offset);
  }
  return v_1;
}
void main() {
  VertexOutputs v_2 = vs_main_inner(uint(gl_VertexID));
  tint_interstage_location0 = v_2.texcoords;
  gl_Position = vec4(v_2.position.x, -(v_2.position.y), ((2.0f * v_2.position.z) - v_2.position.w), v_2.position.w);
  gl_PointSize = 1.0f;
}
//
// fs_main
//
#version 310 es
precision highp float;
precision highp int;

bool continue_execution = true;
layout(location = 0) in vec2 tint_interstage_location0;
layout(location = 0) out vec4 fs_main_loc0_Output;
vec4 fs_main_inner(vec2 texcoord) {
  vec2 clampedTexcoord = clamp(texcoord, vec2(0.0f), vec2(1.0f));
  if (!(all(equal(clampedTexcoord, texcoord)))) {
    continue_execution = false;
  }
  vec4 srcColor = vec4(0.0f);
  if (!(continue_execution)) {
    discard;
  }
  return srcColor;
}
void main() {
  fs_main_loc0_Output = fs_main_inner(tint_interstage_location0);
}
