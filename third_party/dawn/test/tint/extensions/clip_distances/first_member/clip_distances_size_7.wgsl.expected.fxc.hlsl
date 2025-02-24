struct VertexOutputs {
  float clipDistance[7];
  float4 position;
};
struct tint_symbol {
  float4 position : SV_Position;
  float4 clip_distance_0 : SV_ClipDistance0;
  float3 clip_distance_1 : SV_ClipDistance1;
};

VertexOutputs main_inner() {
  VertexOutputs tint_symbol_1 = {(float[7])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return tint_symbol_1;
}

tint_symbol main() {
  VertexOutputs inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  float tmp_inner_clip_distances[7] = inner_result.clipDistance;
  wrapper_result.clip_distance_0[0u] = tmp_inner_clip_distances[0u];
  wrapper_result.clip_distance_0[1u] = tmp_inner_clip_distances[1u];
  wrapper_result.clip_distance_0[2u] = tmp_inner_clip_distances[2u];
  wrapper_result.clip_distance_0[3u] = tmp_inner_clip_distances[3u];
  wrapper_result.clip_distance_1[0u] = tmp_inner_clip_distances[4u];
  wrapper_result.clip_distance_1[1u] = tmp_inner_clip_distances[5u];
  wrapper_result.clip_distance_1[2u] = tmp_inner_clip_distances[6u];
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
