//------------------------------------------------------------------------------
// Bloom
//------------------------------------------------------------------------------

vec3 bloom(vec3 color) {
    highp vec2 uv = variable_vertex.xy;
    vec3 blurred = textureLod(materialParams_bloomBuffer, uv, 0.0).rgb;
    color = blurred * materialParams.bloom.x + color * materialParams.bloom.y;

    if (materialParams.bloom.z > 0.0) {
        float dirtIntensity = materialParams.bloom.z;
        vec3 dirt = textureLod(materialParams_dirtBuffer, uv, 0.0).rgb;
        color += blurred * dirt * dirtIntensity;
    }

    return color;
}
