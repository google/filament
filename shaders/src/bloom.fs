//------------------------------------------------------------------------------
// Bloom
//------------------------------------------------------------------------------

vec3 bloom(const vec3 color) {
    highp vec2 uv = variable_vertex.xy;
    vec3 blurred = textureLod(materialParams_bloomBuffer, uv, 0.0).rgb;
    return blurred * materialParams.bloom.x + color * materialParams.bloom.y;
}
