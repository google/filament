//------------------------------------------------------------------------------
// Varyings
//------------------------------------------------------------------------------

LAYOUT_LOCATION(4) VARYING highp vec4 vertex_worldPosition;

#if defined(HAS_ATTRIBUTE_TANGENTS)
LAYOUT_LOCATION(5) SHADING_INTERPOLATION VARYING mediump vec3 vertex_worldNormal;
#if defined(MATERIAL_NEEDS_TBN)
LAYOUT_LOCATION(6) SHADING_INTERPOLATION VARYING mediump vec4 vertex_worldTangent;
#endif
#endif

LAYOUT_LOCATION(7) VARYING highp vec4 vertex_position;

#if defined(HAS_ATTRIBUTE_COLOR)
LAYOUT_LOCATION(9) VARYING mediump vec4 vertex_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(10) VARYING highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(10) VARYING highp vec4 vertex_uv01;
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
LAYOUT_LOCATION(11) VARYING highp vec4 vertex_lightSpacePosition;
#endif

// Note that fragColor is an output and is not declared here; see main.fs and depth_main.fs
