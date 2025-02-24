# ANGLE RendererGL feature support
Documentation of OpenGL ES and EGL features, caps and formats and required extensions.

## OpenGL ES Feature Support
|Feature|OpenGL version|OpenGL extension|OpenGL ES version|OpenGL ES extension|Notes|
|---|---|---|---|---|---|
|Framebuffer/renderbuffer objects|3.0|[GL_EXT_framebuffer_object](https://www.opengl.org/registry/specs/EXT/framebuffer_object.txt)|2.0|--|Can potentially be emulated with Pbuffers but realistically this extension is always required.|
|Blit framebuffer|3.0|[GL_EXT_framebuffer_blit](https://www.opengl.org/registry/specs/EXT/framebuffer_blit.txt)|3.0|[GL_ANGLE_framebuffer_blit](https://www.khronos.org/registry/gles/extensions/ANGLE/ANGLE_framebuffer_blit.txt) or [GL_NV_framebuffer_blit](https://www.khronos.org/registry/gles/extensions/NV/NV_framebuffer_blit.txt)||
|Multisampling|3.0|[GL_EXT_framebuffer_multisample](https://www.opengl.org/registry/specs/EXT/framebuffer_multisample.txt)|3.0|[GL_EXT_multisampled_render_to_texture](https://www.khronos.org/registry/gles/extensions/EXT/EXT_multisampled_render_to_texture.txt) or [GL_ANGLE_framebuffer_multisample](https://www.khronos.org/registry/gles/extensions/ANGLE/ANGLE_framebuffer_multisample.txt)||
|Depth textures|3.0|[GL_ARB_depth_texture](https://www.opengl.org/registry/specs/ARB/depth_texture.txt)|3.0|[GL_OES_depth_texture](https://www.khronos.org/registry/gles/extensions/OES/OES_depth_texture.txt) or [GL_ANGLE_depth_texture](https://www.khronos.org/registry/gles/extensions/ANGLE/ANGLE_depth_texture.txt)
|Draw buffers (MRT)|2.0?|[GL_ARB_draw_buffers](https://www.opengl.org/registry/specs/ARB/draw_buffers.txt) or [GL_EXT_draw_buffers2](https://www.opengl.org/registry/specs/EXT/draw_buffers2.txt)|3.0|[GL_EXT_draw_buffers](https://www.khronos.org/registry/gles/extensions/EXT/EXT_draw_buffers.txt)||
|3D textures|1.2|[GL_EXT_texture3D](https://www.opengl.org/registry/specs/EXT/texture3D.txt)|3.0|[GL_OES_texture_3D](https://www.khronos.org/registry/gles/extensions/OES/OES_texture_3D.txt)||
|Array textures|3.0|[GL_EXT_texture_array](https://www.opengl.org/registry/specs/EXT/texture_array.txt)|3.0|--||
|Texture storage|4.2|[GL_EXT_texture_storage](https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_storage.txt)|3.0|[GL_EXT_texture_storage](https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_storage.txt)|Can be emulated with TexImage calls.|
|Uniform buffer object|3.1|[GL_ARB_uniform_buffer_object](https://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt)|3.0|--||
|Sync objects|3.2|[GL_ARB_sync](https://www.opengl.org/registry/specs/ARB/sync.txt)|3.0|--||
|Fence objects|--|[GL_NV_fence](https://www.opengl.org/registry/specs/NV/fence.txt)|--|[GL_NV_fence](https://www.opengl.org/registry/specs/NV/fence.txt)||
|MapBuffer|1.5|--|--|[GL_OES_mapbuffer](https://www.khronos.org/registry/gles/extensions/OES/OES_mapbuffer.txt)||
|MapBufferRange|3.0|[GL_ARB_map_buffer_range](https://www.opengl.org/registry/specs/ARB/map_buffer_range.txt)|3.0|[GL_EXT_map_buffer_range](https://www.khronos.org/registry/gles/extensions/EXT/EXT_map_buffer_range.txt)||
|Transform feedback|3.0|[GL_EXT_transform_feedback](GL_EXT_transform_feedback) or [GL_EXT_transform_feedback2](http://developer.download.nvidia.com/opengl/specs/GL_EXT_transform_feedback2.txt) or [GL_ARB_transform_feedback3](https://www.opengl.org/registry/specs/ARB/transform_feedback3.txt)|3.0|--||
|Sampler object|3.3|[GL_ARB_sampler_objects](https://www.opengl.org/registry/specs/ARB/sampler_objects.txt)|3.0|--||
|Occlusion query|1.5|[GL_ARB_occlusion_query](https://www.opengl.org/registry/specs/ARB/occlusion_query.txt)|2.0|--||
|Timer query|3.3|[GL_ARB_timer_query](https://www.opengl.org/registry/specs/ARB/timer_query.txt)|--|[GL_EXT_disjoint_timer_query](https://www.khronos.org/registry/gles/extensions/EXT/EXT_disjoint_timer_query.txt)||
|Vertex array object|3.0|[GL_ARB_vertex_array_object](https://www.opengl.org/registry/specs/ARB/vertex_array_object.txt)|3.0|[GL_OES_vertex_array_object](https://www.khronos.org/registry/gles/extensions/OES/OES_vertex_array_object.txt)|Can be emulated but costsmany extra API calls.  Virtualized contexts also require some kind of emulation of the default attribute state.|
|Anisotropic filtering|--|[GL_EXT_texture_filter_anisotropic](https://www.opengl.org/registry/specs/EXT/texture_filter_anisotropic.txt)|--|[GL_EXT_texture_filter_anisotropic](https://www.opengl.org/registry/specs/EXT/texture_filter_anisotropic.txt)|Ubiquitous extension.|

## OpenGL ES Caps
|Cap(s)|OpenGL version|OpenGL extension|OpenGL ES version|OpenGL ES extension|Notes|
|---|---|---|---|---|---|
|GL_MAX_ELEMENT_INDEX|4.3|[GL_ARB_ES3_compatibility](https://www.opengl.org/registry/specs/ARB/ES3_compatibility.txt)|3.0|--|Seems pretty safe to use an arbitrary limit, all implementations tested return 0xFFFFFFFF.|
|GL_MAX_3D_TEXTURE_SIZE|1.2|[GL_EXT_texture3D](https://www.opengl.org/registry/specs/EXT/texture3D.txt)|3.0|[GL_OES_texture_3D](https://www.khronos.org/registry/gles/extensions/OES/OES_texture_3D.txt)||
|GL_MAX_TEXTURE_SIZE|1.0|--|2.0|--||
|GL_MAX_CUBE_MAP_TEXTURE_SIZE|1.3|--|2.0||
|GL_MAX_ARRAY_TEXTURE_LAYERS|3.0|[GL_EXT_texture_array](https://www.opengl.org/registry/specs/EXT/texture_array.txt)|3.0|--||
|GL_MAX_TEXTURE_LOD_BIAS|1.5|[GL_EXT_texture_lod_bias](https://www.opengl.org/registry/specs/EXT/texture_lod_bias.txt)|3.0|--||
|GL_MAX_RENDERBUFFER_SIZE GL_MAX_COLOR_ATTACHMENTS|3.0|[GL_EXT_framebuffer_object](https://www.opengl.org/registry/specs/EXT/framebuffer_object.txt) |2.0|--||
|GL_MAX_DRAW_BUFFERS|2.0?|[GL_ARB_draw_buffers](https://www.opengl.org/registry/specs/ARB/draw_buffers.txt) or [GL_EXT_draw_buffers2](https://www.opengl.org/registry/specs/EXT/draw_buffers2.txt)|3.0|[GL_EXT_draw_buffers](https://www.khronos.org/registry/gles/extensions/EXT/EXT_draw_buffers.txt)||
|GL_MAX_VIEWPORT_DIMS|1.0|--|2.0|--||
|GL_ALIASED_POINT_SIZE_RANGE|1.0?|--|2.0|--||
|GL_ALIASED_LINE_WIDTH_RANGE|1.2|--|2.0|--||
|GL_ALIASED_LINE_WIDTH_RANGE|1.2|--|2.0|--||
|GL_MAX_ELEMENTS_INDICES|1.2|--|3.0|--||
|GL_MAX_ELEMENTS_VERTICES|1.2|--|3.0|--||
|Shader format precision (glGetShaderPrecisionFormat)|4.1|[GL_ARB_ES2_compatibility](https://www.opengl.org/registry/specs/ARB/ES2_compatibility.txt)|2.0|--|Can use reasonable default values (IEEE float and twos complement integer).|
|GL_MAX_VERTEX_ATTRIBS|2.0|--|2.0|--||
|GL_MAX_VERTEX_UNIFORM_COMPONENTS|2.0|--|2.0|--||
|GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS|2.0|--|2.0|--||
|GL_MAX_VERTEX_UNIFORM_VECTORS GL_MAX_FRAGMENT_UNIFORM_VECTORS|4.1|[GL_ARB_ES2_compatibility](https://www.opengl.org/registry/specs/ARB/ES2_compatibility.txt)|2.0|--|Defined as GL_MAX_VERTEX_UNIFORM_COMPONENTS / 4 and GL_MAX_FRAGMENT_UNIFORM_COMPONENTS / 4.  Can simply use those values when the cap is not available.|
|GL_MAX_VERTEX_UNIFORM_BLOCKS GL_MAX_FRAGMENT_UNIFORM_BLOCKS GL_MAX_UNIFORM_BUFFER_BINDINGS GL_MAX_UNIFORM_BLOCK_SIZE GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT GL_MAX_COMBINED_UNIFORM_BLOCKS GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS |3.1|[GL_ARB_uniform_buffer_object](https://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt)|3.0|--||
|GL_MAX_VERTEX_OUTPUT_COMPONENTS|3.2|--|3.0|--|Doesn't seem to be a desktop extension for this cap, it may be possible to use the minimum ES3 value (64) if lower than GL 3.2.|
|GL_MAX_FRAGMENT_UNIFORM_COMPONENTS|2.0|--|2.0|--||
|GL_MAX_TEXTURE_IMAGE_UNITS|2.0|--|2.0|--||
|GL_MAX_FRAGMENT_INPUT_COMPONENTS|3.2|--|3.0|--|Doesn't seem to be a desktop extension for this cap either, it may be possible to use the minimum ES3 value (60) if lower than GL 3.2.|
|GL_MIN_PROGRAM_TEXEL_OFFSET GL_MAX_PROGRAM_TEXEL_OFFSET|3.0|--|3.0|--|Could potentially be emulated in the shader by adding the offsets in normalized texture coordinates before sampling.|
|GL_MAX_VARYING_COMPONENTS|3.0|[GL_ARB_ES3_compatibility](https://www.opengl.org/registry/specs/ARB/ES3_compatibility.txt)|3.0|--|Was depricated in the OpenGL core spec but re-added in GL_ARB_ES3_compatibility|
|GL_MAX_VARYING_VECTORS|4.1|[GL_ARB_ES2_compatibility](https://www.opengl.org/registry/specs/ARB/ES2_compatibility.txt)|2.0|--|Defined as GL_MAX_VARYING_COMPONENTS / 4.|
|GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS|3.0|[GL_EXT_transform_feedback](GL_EXT_transform_feedback) or [GL_EXT_transform_feedback2](http://developer.download.nvidia.com/opengl/specs/GL_EXT_transform_feedback2.txt) or [GL_ARB_transform_feedback3](https://www.opengl.org/registry/specs/ARB/transform_feedback3.txt)|3.0|--
|GL_MAX_SAMPLES|3.0|[GL_EXT_framebuffer_multisample](https://www.opengl.org/registry/specs/EXT/framebuffer_multisample.txt)|3.0|[GL_EXT_multisampled_render_to_texture](https://www.khronos.org/registry/gles/extensions/EXT/EXT_multisampled_render_to_texture.txt) or [GL_ANGLE_framebuffer_multisample](https://www.khronos.org/registry/gles/extensions/ANGLE/ANGLE_framebuffer_multisample.txt)||
|GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT|--|[GL_EXT_texture_filter_anisotropic](https://www.opengl.org/registry/specs/EXT/texture_filter_anisotropic.txt)|--|[GL_EXT_texture_filter_anisotropic](https://www.opengl.org/registry/specs/EXT/texture_filter_anisotropic.txt)|Ubiquitous extension.|
|IMPLEMENTATION_COLOR_READ_FORMAT IMPLEMENTATION_COLOR_READ_TYPE|--|[GL_ARB_ES2_compatibility](https://www.opengl.org/registry/specs/ARB/ES2_compatibility.txt)|2.0|--|Desktop GL doesn't as many limitations as ES for ReadPixels, can either always return GL_RGBA/GL_UNSIGNED_BYTE or return the format and type of the read buffer.|

##OpenGL ES Formats (TODO)
|Format|OpenGL version|OpenGL extension|OpenGL ES version|OpenGL ES extension|Notes|
|---|---|---|---|---|---|
|GL_RGBA8<br>GL_RGB8 |1.0|--|3.0|[GL_OES_rgb8_rgba8](https://www.khronos.org/registry/gles/extensions/OES/OES_rgb8_rgba8.txt)||

## ESSL Features (TODO)
|Feature|GLSL version|Notes|
|---|---|---|
|Unsigned integers|1.30||
|Pack layout std140|1.40||

## ESSL3 Builtins
Builtins that are added going from ESSL1 to ESSL3.

|Function|GLSL version|Extension|Notes|
|---|---|---|---|
|sinh<br>cosh<br>tanh<br>asinh<br>acosh|1.30|||
|atanh|1.10|||
|abs (genIType)|1.30|[GL_EXT_gpu_shader4](https://www.opengl.org/registry/specs/EXT/gpu_shader4.txt)||
|sign (genIType)|1.50|[GL_EXT_gpu_shader4](https://www.opengl.org/registry/specs/EXT/gpu_shader4.txt)|Can be emulated easily.|
|trunc|1.30|||
|round<br>roundEven|1.30|||
|min (genIType, genUType)<br>max (genIType, genUType)<br>clamp (genIType, genUType)|1.30||
|mix (genBType)|4.50|[GL_EXT_shader_integer_mix](https://www.opengl.org/registry/specs/EXT/shader_integer_mix.txt)|Should be possible to emulate with a ternery operation.|
|modf|1.30|||
|isnan|1.30|||
|isinf|1.10|||
|floatBitsToInt<br>floatBitsToUint<br>intBitsToFloat<br>uintBitsToFloat|3.30|[GL_ARB_shader_bit_encoding](https://www.opengl.org/registry/specs/ARB/shader_bit_encoding.txt) or [ARB_gpu_shader5](https://www.opengl.org/registry/specs/ARB/gpu_shader5.txt)||
|packSnorm2x16<br>packHalf2x16<br>unpackSnorm2x16<br>unpackHalf2x16|4.20|[GL_ARB_shading_language_packing](https://www.opengl.org/registry/specs/ARB/shading_language_packing.txt)|Can be emulated using bit casting functions.|
|packUnorm2x16<br>unpackUnorm2x16|4.10|[GL_ARB_shading_language_packing](https://www.opengl.org/registry/specs/ARB/shading_language_packing.txt)|Can be emulated using bit casting functions.|
|matrixCompMult (NxM matrices)|1.10|||
|outerProduct|1.20|||
|transpose|1.20|||
|determinant|1.50||Can be emulated.|
|inverse|1.40||Can be emulated.|
|lessThan (uvec)<br>lessThanEqual (uvec)<br>greaterThan (uvec)<br>greaterThanEqual (uvec)<br>equal (uvec)<br>notEqual (uvec)|1.30|||
|texture<br>textureProj<br>textureLod<br>textureOffset<br>textureProjOffset<br>textureLodOffset<br>textureProjLod<br>textureProjLodOffset<br>texelFetch<br>texelFetchOffset<br>textureGrad<br>textureGradOffset<br>textureProjGrad<br>textureProjGradOffset<br>textureSize|1.30||Equivalent to texture2D, textureCube, etc|
|dFdx<br>dFdy<br>fwidth|1.10||

## EGL Feature Support (TODO)
|Feature|EGL version|EGL extension|WGL core|WGL extension|GLX version|GLX extensions|Notes|
|---|---|---|---|---|---|---|---|
|Pbuffers|||No|[WGL_ARB_pbuffer](https://www.opengl.org/registry/specs/ARB/wgl_pbuffer.txt)||||
|BindTexImage|||No|[WGL_ARB_render_texture](https://www.opengl.org/registry/specs/ARB/wgl_render_texture.txt)|||Possibly to emulate with OpenGL textures but not strictly required, it is possible only export EGL configs without EGL_BIND_TO_TEXTURE_RGB and EGL_BIND_TO_TEXTURE_RGBA. Bindable pbuffers may be required by Chrome though.|
|Pixmaps||||||||
|Swap control|||No|[WGL_EXT_swap_control](https://www.opengl.org/registry/specs/EXT/wgl_swap_control.txt)|No|[GLX_EXT_swap_control](https://www.opengl.org/registry/specs/EXT/swap_control.txt)||
