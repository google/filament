//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}
vec4 vertex_main_inner() {
  asinh_468a48();
  return vec4(0.0f);
}
void main() {
  vec4 v = vertex_main_inner();
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}
void main() {
  asinh_468a48();
}
//
// rgba32uintin
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  asinh_468a48();
}
//
// vs_main
//
#version 310 es


struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  uint tint_pad_0;
  vec3 up;
  uint tint_pad_1;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

layout(binding = 5, std140)
uniform v_render_params_block_ubo {
  RenderParams inner;
} v;
layout(location = 0) in vec3 vs_main_loc0_Input;
layout(location = 1) in vec4 vs_main_loc1_Input;
layout(location = 2) in vec2 vs_main_loc2_Input;
layout(location = 0) out vec4 tint_interstage_location0;
layout(location = 1) out vec2 tint_interstage_location1;
VertexOutput vs_main_inner(VertexInput v_1) {
  vec3 quad_pos = (mat2x3(v.inner.right, v.inner.up) * v_1.quad_pos);
  vec3 position = (v_1.position - (quad_pos + 0.00999999977648258209f));
  VertexOutput v_2 = VertexOutput(vec4(0.0f), vec4(0.0f), vec2(0.0f));
  mat4 v_3 = v.inner.modelViewProjectionMatrix;
  v_2.position = (v_3 * vec4(position, 1.0f));
  v_2.color = v_1.color;
  v_2.quad_pos = v_1.quad_pos;
  return v_2;
}
void main() {
  VertexOutput v_4 = vs_main_inner(VertexInput(vs_main_loc0_Input, vs_main_loc1_Input, vs_main_loc2_Input));
  gl_Position = vec4(v_4.position.x, -(v_4.position.y), ((2.0f * v_4.position.z) - v_4.position.w), v_4.position.w);
  tint_interstage_location0 = v_4.color;
  tint_interstage_location1 = v_4.quad_pos;
  gl_PointSize = 1.0f;
}
//
// simulate
//
#version 310 es


struct SimulationParams {
  float deltaTime;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec4 seed;
};

struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
  uint tint_pad_0;
  uint tint_pad_1;
};

vec2 rand_seed = vec2(0.0f);
layout(binding = 0, std140)
uniform sim_params_block_1_ubo {
  SimulationParams inner;
} v;
layout(binding = 1, std430)
buffer Particles_1_ssbo {
  Particle particles[];
} data;
void tint_store_and_preserve_padding(uint target_indices[1], Particle value_param) {
  data.particles[target_indices[0u]].position = value_param.position;
  data.particles[target_indices[0u]].lifetime = value_param.lifetime;
  data.particles[target_indices[0u]].color = value_param.color;
  data.particles[target_indices[0u]].velocity = value_param.velocity;
}
void simulate_inner(uvec3 GlobalInvocationID) {
  vec2 v_1 = v.inner.seed.xy;
  vec2 v_2 = (v_1 * vec2(GlobalInvocationID.xy));
  rand_seed = (v_2 * v.inner.seed.zw);
  uint idx = GlobalInvocationID.x;
  uint v_3 = min(idx, (uint(data.particles.length()) - 1u));
  Particle particle = data.particles[v_3];
  uint v_4 = min(idx, (uint(data.particles.length()) - 1u));
  Particle v_5 = particle;
  tint_store_and_preserve_padding(uint[1](v_4), v_5);
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  simulate_inner(gl_GlobalInvocationID);
}
//
// export_level
//
#version 310 es


struct UBO {
  uint width;
};

layout(binding = 3, std140)
uniform ubo_block_1_ubo {
  UBO inner;
} v;
layout(binding = 4, std430)
buffer Buffer_1_ssbo {
  float weights[];
} buf_in;
layout(binding = 5, std430)
buffer Buffer_2_ssbo {
  float weights[];
} buf_out;
layout(binding = 7, rgba8) uniform highp writeonly image2D tex_out;
float tint_float_modulo(float x, float y) {
  return (x - (y * trunc((x / y))));
}
void export_level_inner(uvec3 coord) {
  if (all(lessThan(coord.xy, uvec2(uvec2(imageSize(tex_out)))))) {
    uint dst_offset = (coord.x << ((coord.y * v.inner.width) & 31u));
    uint src_offset = ((coord.x - 2u) + ((coord.y >> (2u & 31u)) * v.inner.width));
    uint v_1 = min((src_offset << (0u & 31u)), (uint(buf_in.weights.length()) - 1u));
    float a = buf_in.weights[v_1];
    uint v_2 = min((src_offset + 1u), (uint(buf_in.weights.length()) - 1u));
    float b = buf_in.weights[v_2];
    uint v_3 = ((src_offset + 1u) + v.inner.width);
    uint v_4 = min(v_3, (uint(buf_in.weights.length()) - 1u));
    float c = buf_in.weights[v_4];
    uint v_5 = ((src_offset + 1u) + v.inner.width);
    uint v_6 = min(v_5, (uint(buf_in.weights.length()) - 1u));
    float d = buf_in.weights[v_6];
    float sum = dot(vec4(a, b, c, d), vec4(1.0f));
    uint v_7 = min(dst_offset, (uint(buf_out.weights.length()) - 1u));
    buf_out.weights[v_7] = tint_float_modulo(sum, 4.0f);
    vec4 probabilities = (vec4(a, (a * b), ((a / b) + c), sum) + max(sum, 0.0f));
    imageStore(tex_out, ivec2(coord.xy), probabilities);
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  export_level_inner(gl_GlobalInvocationID);
}
