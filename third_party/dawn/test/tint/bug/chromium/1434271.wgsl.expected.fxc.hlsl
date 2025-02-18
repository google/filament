SKIP: INVALID

float tint_trunc(float param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float16_t tint_sinh(float16_t x) {
  return log((x + sqrt(((x * x) + float16_t(1.0h)))));
}

void asinh_468a48() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t res = tint_sinh(arg_0);
}

struct tint_symbol_4 {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asinh_468a48();
  return (0.0f).xxxx;
}

tint_symbol_4 vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol_4 wrapper_result = (tint_symbol_4)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asinh_468a48();
  return;
}

[numthreads(1, 1, 1)]
void rgba32uintin() {
  asinh_468a48();
  return;
}

static float2 rand_seed = float2(0.0f, 0.0f);

cbuffer cbuffer_render_params : register(b5) {
  uint4 render_params[6];
};

struct VertexInput {
  float3 position;
  float4 color;
  float2 quad_pos;
};
struct VertexOutput {
  float4 position;
  float4 color;
  float2 quad_pos;
};
struct tint_symbol_6 {
  float3 position : TEXCOORD0;
  float4 color : TEXCOORD1;
  float2 quad_pos : TEXCOORD2;
};
struct tint_symbol_7 {
  float4 color : TEXCOORD0;
  float2 quad_pos : TEXCOORD1;
  float4 position : SV_Position;
};

float4x4 render_params_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(render_params[scalar_offset / 4]), asfloat(render_params[scalar_offset_1 / 4]), asfloat(render_params[scalar_offset_2 / 4]), asfloat(render_params[scalar_offset_3 / 4]));
}

VertexOutput vs_main_inner(VertexInput tint_symbol) {
  float3 quad_pos = mul(tint_symbol.quad_pos, float2x3(asfloat(render_params[4].xyz), asfloat(render_params[5].xyz)));
  float3 position = (tint_symbol.position - (quad_pos + 0.00999999977648258209f));
  VertexOutput tint_symbol_1 = (VertexOutput)0;
  tint_symbol_1.position = mul(float4(position, 1.0f), render_params_load_1(0u));
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  return tint_symbol_1;
}

tint_symbol_7 vs_main(tint_symbol_6 tint_symbol_5) {
  VertexInput tint_symbol_12 = {tint_symbol_5.position, tint_symbol_5.color, tint_symbol_5.quad_pos};
  VertexOutput inner_result_1 = vs_main_inner(tint_symbol_12);
  tint_symbol_7 wrapper_result_1 = (tint_symbol_7)0;
  wrapper_result_1.position = inner_result_1.position;
  wrapper_result_1.color = inner_result_1.color;
  wrapper_result_1.quad_pos = inner_result_1.quad_pos;
  return wrapper_result_1;
}

struct Particle {
  float3 position;
  float lifetime;
  float4 color;
  float2 velocity;
};

cbuffer cbuffer_sim_params : register(b0) {
  uint4 sim_params[2];
};
RWByteAddressBuffer data : register(u1);
Texture1D<float4> tint_symbol_2 : register(t2);

struct tint_symbol_9 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

Particle data_load(uint offset) {
  Particle tint_symbol_13 = {asfloat(data.Load3((offset + 0u))), asfloat(data.Load((offset + 12u))), asfloat(data.Load4((offset + 16u))), asfloat(data.Load2((offset + 32u)))};
  return tint_symbol_13;
}

void data_store(uint offset, Particle value) {
  data.Store3((offset + 0u), asuint(value.position));
  data.Store((offset + 12u), asuint(value.lifetime));
  data.Store4((offset + 16u), asuint(value.color));
  data.Store2((offset + 32u), asuint(value.velocity));
}

void simulate_inner(uint3 GlobalInvocationID) {
  rand_seed = ((asfloat(sim_params[1]).xy * float2(GlobalInvocationID.xy)) * asfloat(sim_params[1]).zw);
  uint idx = GlobalInvocationID.x;
  Particle particle = data_load((48u * idx));
  data_store((48u * idx), particle);
}

[numthreads(64, 1, 1)]
void simulate(tint_symbol_9 tint_symbol_8) {
  simulate_inner(tint_symbol_8.GlobalInvocationID);
  return;
}

cbuffer cbuffer_ubo : register(b3) {
  uint4 ubo[1];
};
ByteAddressBuffer buf_in : register(t4);
RWByteAddressBuffer buf_out : register(u5);
Texture2D<float4> tex_in : register(t6);
RWTexture2D<float4> tex_out : register(u7);

float tint_float_mod(float lhs, float rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

struct tint_symbol_11 {
  uint3 coord : SV_DispatchThreadID;
};

void export_level_inner(uint3 coord) {
  uint2 tint_tmp;
  tex_out.GetDimensions(tint_tmp.x, tint_tmp.y);
  if (all((coord.xy < uint2(tint_tmp)))) {
    uint dst_offset = (coord.x << ((coord.y * ubo[0].x) & 31u));
    uint src_offset = ((coord.x - 2u) + ((coord.y >> 2u) * ubo[0].x));
    float a = asfloat(buf_in.Load((4u * (src_offset << 0u))));
    float b = asfloat(buf_in.Load((4u * (src_offset + 1u))));
    float c = asfloat(buf_in.Load((4u * ((src_offset + 1u) + ubo[0].x))));
    float d = asfloat(buf_in.Load((4u * ((src_offset + 1u) + ubo[0].x))));
    float sum = dot(float4(a, b, c, d), (1.0f).xxxx);
    buf_out.Store((4u * dst_offset), asuint(tint_float_mod(sum, 4.0f)));
    float4 probabilities = (float4(a, (a * b), ((a / b) + c), sum) + max(sum, 0.0f));
    tex_out[int2(coord.xy)] = probabilities;
  }
}

[numthreads(64, 1, 1)]
void export_level(tint_symbol_11 tint_symbol_10) {
  export_level_inner(tint_symbol_10.coord);
  return;
}
FXC validation failure:
<scrubbed_path>(5,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
