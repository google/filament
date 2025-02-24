# gl_ClipDistance extension support in Metal back-end

OpenGL GLSL's `gl_ClipDistance` is equivalent to `[[clip_distance]]` attribute in the Metal Shading
Language. However, OpenGL supports disabling/enabling individual `gl_ClipDistance[i]` on the API
level side. Writing to `gl_ClipDistance[i]` in shader will be ignored if it is disabled. Metal
doesn't have any equivalent API to disable/enable the writing, though writing to a `clip_distance`
variable automatically enables it.

To emulate this enabling/disabling API, the Metal back-end uses a similar implementation as what
[Vulkan back-end does](../../vulkan/doc/APPLE_clip_distance.md). Please refer to that document for
more details.
