# ANGLE Texture Sharing

Available only on the OpenGL ES backend when running on top of ANGLE.
When a device is created with this extension, it is added to the `EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE` which is a display-wide share context, but only for texturs.

When enabled, `dawn::native::opengl::ExternalImageDescriptorGLTexture` and `dawn::native::opengl::WrapExternalGLTexture` become available which can import a GL texture into a `wgpu::Device` with a GL texture ID.

See the spec for [`EGL_ANGLE_display_texture_share_group`](https://chromium.googlesource.com/angle/angle/+/refs/heads/main/extensions/EGL_ANGLE_display_texture_share_group.txt).
