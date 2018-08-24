layout(location = LOCATION_POSITION) in vec4 mesh_position;

#if defined(HAS_ATTRIBUTE_TANGENTS)
layout(location = LOCATION_TANGENTS) in vec4 mesh_tangents;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
layout(location = LOCATION_COLOR) in vec4 mesh_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0)
layout(location = LOCATION_UV0) in vec2 mesh_uv0;
#endif

#if defined(HAS_ATTRIBUTE_UV1)
layout(location = LOCATION_UV1) in vec2 mesh_uv1;
#endif

#if defined(HAS_ATTRIBUTE_BONE_INDICES)
layout(location = LOCATION_BONE_INDICES) in uvec4 mesh_bone_indices;
#endif

#if defined(HAS_ATTRIBUTE_BONE_WEIGHTS)
layout(location = LOCATION_BONE_WEIGHTS) in vec4 mesh_bone_weights;
#endif

LAYOUT_LOCATION(4) out HIGHP vec3 vertex_worldPosition;
#if defined(HAS_ATTRIBUTE_TANGENTS)
LAYOUT_LOCATION(5) SHADING_INTERPOLATION out MEDIUMP vec3 vertex_worldNormal;
#if defined(MATERIAL_HAS_ANISOTROPY) || defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
LAYOUT_LOCATION(6) SHADING_INTERPOLATION out MEDIUMP vec3 vertex_worldTangent;
LAYOUT_LOCATION(7) SHADING_INTERPOLATION out MEDIUMP vec3 vertex_worldBitangent;
#endif
#if defined(GEOMETRIC_SPECULAR_AA_NORMAL)
LAYOUT_LOCATION(8) SHADING_INTERPOLATION centroid out vec3 vertex_worldNormalCentroid;
#endif
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
LAYOUT_LOCATION(9) out MEDIUMP vec4 vertex_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0)
LAYOUT_LOCATION(10) out HIGHP vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(10) out HIGHP vec4 vertex_uv01;
#endif

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
LAYOUT_LOCATION(11) out HIGHP vec4 vertex_lightSpacePosition;
#endif
