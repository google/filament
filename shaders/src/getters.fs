//------------------------------------------------------------------------------
// Input access
//------------------------------------------------------------------------------

#if defined(HAS_ATTRIBUTE_COLOR)
/** @public-api */
vec4 getColor() {
    return vertex_color;
}
#endif

#if defined(HAS_ATTRIBUTE_UV0)
/** @public-api */
vec2 getUV0() {
    return vertex_uv01.xy;
}
#endif

#if defined(HAS_ATTRIBUTE_UV1)
/** @public-api */
vec2 getUV1() {
    return vertex_uv01.zw;
}
#endif

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
HIGHP vec3 getLightSpacePosition() {
    return vertex_lightSpacePosition.xyz * (1.0 / vertex_lightSpacePosition.w);
}
#endif

#if defined(BLEND_MODE_MASKED)
float getMaskThreshold() {
    return materialParams._maskThreshold;
}
#endif

#if defined(MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY)
bool isDoubleSided() {
    return materialParams._doubleSided;
}
#endif
