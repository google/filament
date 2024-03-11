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

#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
LAYOUT_LOCATION(8) flat VARYING highp int instance_index;
highp int logical_instance_index;
#endif

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

#if defined(VARIANT_HAS_STEREO) && defined(FILAMENT_STEREO_INSTANCED)
#if defined(GL_ES) && defined(FILAMENT_GLSLANG)
// On ES, gl_ClipDistance is not a built-in, so we have to rely on EXT_clip_cull_distance
// However, this extension is not supported by glslang, so we instead write to
// filament_gl_ClipDistance, which will get decorated at the SPIR-V stage to refer to the built-in.
// The location here does not matter, so long as it doesn't conflict with others.
LAYOUT_LOCATION(100) out float filament_gl_ClipDistance[2];
#define FILAMENT_CLIPDISTANCE filament_gl_ClipDistance
#else
// If we're on Desktop GL (or not running shaders through glslang), we're free to use gl_ClipDistance
#define FILAMENT_CLIPDISTANCE gl_ClipDistance
#endif // GL_ES && FILAMENT_GLSLANG
#endif // VARIANT_HAS_STEREO
