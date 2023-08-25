//------------------------------------------------------------------------------
// Vignette
//------------------------------------------------------------------------------

// Applies a vignette effect
// color: source color to apply the vignette on top of
// uv: viewport coordinates
// vignette: pre-computed parameters midPoint, radius, aspect and feather
// vignetteColor: color of the vignette effect
vec3 vignette(const vec3 color, const highp vec2 uv, const vec4 vignette, const vec4 vignetteColor) {
    float midPoint = vignette.x;
    float radius = vignette.y;
    float aspect = vignette.z;
    float feather = vignette.w;

    vec2 distance = abs(uv - 0.5) * midPoint;
    distance.x *= aspect;
    distance = pow(saturate(distance), vec2(radius));

    float amount = pow(saturate(1.0 - dot(distance, distance)), feather * 5.0);
    return color * mix(vignetteColor.rgb, vec3(1.0), amount);
}
