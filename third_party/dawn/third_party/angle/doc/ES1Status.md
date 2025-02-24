# ES1 Status

ES1 is implemented entirely in the front-end using features from GLES3.0.  Therefore, every backend
with GLES3 support (i.e. everything other than D3D9) supports GLES1 as well.

ANGLE passes GLES1's `MustPass`, however there are known missing features.

| Features                             | Status                                       | Backends    |
|:-------------------------------------|:---------------------------------------------|:------------|
| Logic Op                             | Implemented through extensions [1](#notes-1) | Vulkan, GL  |
| Palette compressed textures          | Emulated with uncompressed format            | Vulkan      |
| [Smooth lines][lines]                | Unimplemented                                | None        |
| [Two-sided lighting][lighting]       | Unimplemented                                | None        |
| [Matrix palette][matrix]             | Unimplemented (optional)                     | None        |

[lines]: http://anglebug.com/42266418
[lighting]: http://anglebug.com/42266170
[matrix]: http://anglebug.com/42266419

### Notes [1]
* Logic op is implemented through the `ANGLE_logic_op` or `EXT_framebuffer_fetch` extensions.
* Currently, these are supported on the Vulkan and GL backends only.
