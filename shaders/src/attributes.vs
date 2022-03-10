//------------------------------------------------------------------------------
// Attributes
//------------------------------------------------------------------------------

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

#if defined(HAS_ATTRIBUTE_CUSTOM0)
layout(location = LOCATION_CUSTOM0) in vec4 mesh_custom0;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM1)
layout(location = LOCATION_CUSTOM1) in vec4 mesh_custom1;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM2)
layout(location = LOCATION_CUSTOM2) in vec4 mesh_custom2;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM3)
layout(location = LOCATION_CUSTOM3) in vec4 mesh_custom3;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM4)
layout(location = LOCATION_CUSTOM4) in vec4 mesh_custom4;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM5)
layout(location = LOCATION_CUSTOM5) in vec4 mesh_custom5;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM6)
layout(location = LOCATION_CUSTOM6) in vec4 mesh_custom6;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM7)
layout(location = LOCATION_CUSTOM7) in vec4 mesh_custom7;
#endif
