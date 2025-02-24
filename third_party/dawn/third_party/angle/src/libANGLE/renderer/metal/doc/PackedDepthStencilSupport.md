# Packed depth stencil support in Metal

Metal has different runtime behaviors when it comes to packed depth stencil format usage.

On macOS, packed depth24stencil8 format is supported (albeit optionally) and if application
wants to use both depth and stencil attachments in the same render pass, these attachments must
point to the same packed depth stencil texture. In other words, it is not permitted to use separate
depth & stencil textures in a same render pass.

iOS simulators and mac Catalyst platforms have the same restrictions as macOS.

On iOS devices, depth24stencil8 format is not available. The only packed format supported is depth32stencil8 which is a 64 bits format (24 bits unused). However, metal runtime allows separate
depth & stencil textures to be attached to one render pass. So technically, one depth32 texture
and one stencil8 texture can be used together.