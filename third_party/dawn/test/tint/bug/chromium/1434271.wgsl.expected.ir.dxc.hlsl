//
// vertex_main
//
struct vertex_main_outputs {
  float4 tint_symbol : SV_Position;
};


void asinh_468a48() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t v = arg_0;
  float16_t res = log((v + sqrt(((v * v) + float16_t(1.0h)))));
}

float4 vertex_main_inner() {
  asinh_468a48();
  return (0.0f).xxxx;
}

vertex_main_outputs vertex_main() {
  vertex_main_outputs v_1 = {vertex_main_inner()};
  return v_1;
}

//
// fragment_main
//

void asinh_468a48() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t v = arg_0;
  float16_t res = log((v + sqrt(((v * v) + float16_t(1.0h)))));
}

void fragment_main() {
  asinh_468a48();
}

//
// rgba32uintin
//

void asinh_468a48() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t v = arg_0;
  float16_t res = log((v + sqrt(((v * v) + float16_t(1.0h)))));
}

[numthreads(1, 1, 1)]
void rgba32uintin() {
  asinh_468a48();
}

//
// vs_main
//
struct VertexOutput {
  float4 position;
  float4 color;
  float2 quad_pos;
};

struct VertexInput {
  float3 position;
  float4 color;
  float2 quad_pos;
};

struct vs_main_outputs {
  float4 VertexOutput_color : TEXCOORD0;
  float2 VertexOutput_quad_pos : TEXCOORD1;
  float4 VertexOutput_position : SV_Position;
};

struct vs_main_inputs {
  float3 VertexInput_position : TEXCOORD0;
  float4 VertexInput_color : TEXCOORD1;
  float2 VertexInput_quad_pos : TEXCOORD2;
};


cbuffer cbuffer_render_params : register(b5) {
  uint4 render_params[6];
};
float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(render_params[(start_byte_offset / 16u)]), asfloat(render_params[((16u + start_byte_offset) / 16u)]), asfloat(render_params[((32u + start_byte_offset) / 16u)]), asfloat(render_params[((48u + start_byte_offset) / 16u)]));
}

VertexOutput vs_main_inner(VertexInput v_1) {
  float3 quad_pos = mul(v_1.quad_pos, float2x3(asfloat(render_params[4u].xyz), asfloat(render_params[5u].xyz)));
  float3 position = (v_1.position - (quad_pos + 0.00999999977648258209f));
  VertexOutput v_2 = (VertexOutput)0;
  float4x4 v_3 = v(0u);
  v_2.position = mul(float4(position, 1.0f), v_3);
  v_2.color = v_1.color;
  v_2.quad_pos = v_1.quad_pos;
  VertexOutput v_4 = v_2;
  return v_4;
}

vs_main_outputs vs_main(vs_main_inputs inputs) {
  VertexInput v_5 = {inputs.VertexInput_position, inputs.VertexInput_color, inputs.VertexInput_quad_pos};
  VertexOutput v_6 = vs_main_inner(v_5);
  vs_main_outputs v_7 = {v_6.color, v_6.quad_pos, v_6.position};
  return v_7;
}

//
// simulate
//
struct Particle {
  float3 position;
  float lifetime;
  float4 color;
  float2 velocity;
};

struct simulate_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


static float2 rand_seed = (0.0f).xx;
cbuffer cbuffer_sim_params : register(b0) {
  uint4 sim_params[2];
};
RWByteAddressBuffer data : register(u1);
void v(uint offset, Particle obj) {
  data.Store3((offset + 0u), asuint(obj.position));
  data.Store((offset + 12u), asuint(obj.lifetime));
  data.Store4((offset + 16u), asuint(obj.color));
  data.Store2((offset + 32u), asuint(obj.velocity));
}

Particle v_1(uint offset) {
  Particle v_2 = {asfloat(data.Load3((offset + 0u))), asfloat(data.Load((offset + 12u))), asfloat(data.Load4((offset + 16u))), asfloat(data.Load2((offset + 32u)))};
  return v_2;
}

void simulate_inner(uint3 GlobalInvocationID) {
  float2 v_3 = asfloat(sim_params[1u]).xy;
  float2 v_4 = (v_3 * float2(GlobalInvocationID.xy));
  rand_seed = (v_4 * asfloat(sim_params[1u]).zw);
  uint idx = GlobalInvocationID.x;
  uint v_5 = 0u;
  data.GetDimensions(v_5);
  Particle particle = v_1((0u + (min(idx, ((v_5 / 48u) - 1u)) * 48u)));
  uint v_6 = 0u;
  data.GetDimensions(v_6);
  Particle v_7 = particle;
  v((0u + (min(idx, ((v_6 / 48u) - 1u)) * 48u)), v_7);
}

[numthreads(64, 1, 1)]
void simulate(simulate_inputs inputs) {
  simulate_inner(inputs.GlobalInvocationID);
}

//
// export_level
//
struct export_level_inputs {
  uint3 coord : SV_DispatchThreadID;
};


cbuffer cbuffer_ubo : register(b3) {
  uint4 ubo[1];
};
ByteAddressBuffer buf_in : register(t4);
RWByteAddressBuffer buf_out : register(u5);
RWTexture2D<float4> tex_out : register(u7);
void export_level_inner(uint3 coord) {
  uint2 v = (0u).xx;
  tex_out.GetDimensions(v.x, v.y);
  if (all((coord.xy < uint2(v)))) {
    uint dst_offset = (coord.x << ((coord.y * ubo[0u].x) & 31u));
    uint src_offset = ((coord.x - 2u) + ((coord.y >> (2u & 31u)) * ubo[0u].x));
    uint v_1 = 0u;
    buf_in.GetDimensions(v_1);
    float a = asfloat(buf_in.Load((0u + (min((src_offset << (0u & 31u)), ((v_1 / 4u) - 1u)) * 4u))));
    uint v_2 = 0u;
    buf_in.GetDimensions(v_2);
    float b = asfloat(buf_in.Load((0u + (min((src_offset + 1u), ((v_2 / 4u) - 1u)) * 4u))));
    uint v_3 = 0u;
    buf_in.GetDimensions(v_3);
    float c = asfloat(buf_in.Load((0u + (min(((src_offset + 1u) + ubo[0u].x), ((v_3 / 4u) - 1u)) * 4u))));
    uint v_4 = 0u;
    buf_in.GetDimensions(v_4);
    float d = asfloat(buf_in.Load((0u + (min(((src_offset + 1u) + ubo[0u].x), ((v_4 / 4u) - 1u)) * 4u))));
    float sum = dot(float4(a, b, c, d), (1.0f).xxxx);
    uint v_5 = 0u;
    buf_out.GetDimensions(v_5);
    uint v_6 = (min(dst_offset, ((v_5 / 4u) - 1u)) * 4u);
    float v_7 = (sum / 4.0f);
    buf_out.Store((0u + v_6), asuint((sum - ((((v_7 < 0.0f)) ? (ceil(v_7)) : (floor(v_7))) * 4.0f))));
    float4 probabilities = (float4(a, (a * b), ((a / b) + c), sum) + max(sum, 0.0f));
    tex_out[int2(coord.xy)] = probabilities;
  }
}

[numthreads(64, 1, 1)]
void export_level(export_level_inputs inputs) {
  export_level_inner(inputs.coord);
}

