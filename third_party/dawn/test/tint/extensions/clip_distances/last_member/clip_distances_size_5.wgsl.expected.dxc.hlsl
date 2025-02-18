struct VertexOutputs {
  float4 position;
  float clipDistance[5];
};
struct tint_symbol {
  float4 position : SV_Position;
  float4 clip_distance_0 : SV_ClipDistance0;
  float clip_distance_1 : SV_ClipDistance1;
};

VertexOutputs main_inner() {
  VertexOutputs tint_symbol_1 = {float4(1.0f, 2.0f, 3.0f, 4.0f), (float[5])0};
  return tint_symbol_1;
}

tint_symbol main() {
  VertexOutputs inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.position = inner_result.position;
  float tmp_inner_clip_distances[5] = inner_result.clipDistance;
  wrapper_result.clip_distance_0[0u] = tmp_inner_clip_distances[0u];
  wrapper_result.clip_distance_0[1u] = tmp_inner_clip_distances[1u];
  wrapper_result.clip_distance_0[2u] = tmp_inner_clip_distances[2u];
  wrapper_result.clip_distance_0[3u] = tmp_inner_clip_distances[3u];
  wrapper_result.clip_distance_1 = tmp_inner_clip_distances[4u];
  return wrapper_result;
}
