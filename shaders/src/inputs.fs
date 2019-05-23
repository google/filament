//------------------------------------------------------------------------------
// Attributes and uniforms
//------------------------------------------------------------------------------

#if !defined(DEPTH_PREPASS)
LAYOUT_LOCATION(4) in highp vec3 vertex_worldPosition;
#endif

#if defined(HAS_ATTRIBUTE_TANGENTS)
LAYOUT_LOCATION(5) SHADING_INTERPOLATION in mediump vec3 vertex_worldNormal;
#if defined(MATERIAL_HAS_ANISOTROPY) || defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
LAYOUT_LOCATION(6) SHADING_INTERPOLATION in mediump vec3 vertex_worldTangent;
LAYOUT_LOCATION(7) SHADING_INTERPOLATION in mediump vec3 vertex_worldBitangent;
#endif
#endif

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

layout(location = 0) out vec4 fragColor;
