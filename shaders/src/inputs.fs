//------------------------------------------------------------------------------
// Attributes and uniforms
//------------------------------------------------------------------------------

vec3 mapRange(vec3 t, float low, float high) {
    return (t - vec3(low)) / (high - low);
}

highp vec3 debugThisColor;
int shouldDebugThisColor = 0;

void debugColor(vec3 color) {
    shouldDebugThisColor = 1;
    debugThisColor = color;
}

void debugColor(vec3 color, float low, float high) {
    shouldDebugThisColor = 1;
    debugThisColor = mapRange(color, low, high);
}

LAYOUT_LOCATION(4) in highp vec4 vertex_worldPosition;

#if defined(HAS_ATTRIBUTE_TANGENTS)
LAYOUT_LOCATION(5) SHADING_INTERPOLATION in mediump vec3 vertex_worldNormal;
#if defined(MATERIAL_NEEDS_TBN)
LAYOUT_LOCATION(6) SHADING_INTERPOLATION in mediump vec4 vertex_worldTangent;
#endif
#endif

LAYOUT_LOCATION(7) in highp vec4 vertex_position;

#if defined(HAS_ATTRIBUTE_COLOR)
LAYOUT_LOCATION(9) in mediump vec4 vertex_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(10) in highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(10) in highp vec4 vertex_uv01;
#endif

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
LAYOUT_LOCATION(11) in highp vec4 vertex_lightSpacePosition;
#endif

// Note that fragColor is an output and is not declared here; see main.fs and depth_main.fs
