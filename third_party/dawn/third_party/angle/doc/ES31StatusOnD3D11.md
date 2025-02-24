# ES31 Status on Direct3D 11

| New Features                                       | Status                       | Limitations |
|:---------------------------------------------------|:-----------------------------|:------------|
| Arrays of arrays (shading language only)           | Fully implemented            | None        |
| Compute shaders                                    | Fully implemented            | None        |
| Explicit uniform location                          | Fully implemented            | None        |
| Framebuffers with no attachments                   | Fully implemented            | None        |
| Indirect draw commands                             | Fully implemented            | None        |
| Multisample formats for immutable textures         | Fully implemented            | None        |
| Program interface queries                          | Fully implemented            | None        |
| Shader bitfield operations (shading language only) | Fully implemented            | None        |
| Shader layout binding (shading language only)      | Fully implemented            | None        |
| Texture gather operations                          | Fully implemented            | None        |
| Vertex attribute binding                           | Fully implemented            | None        |
| Atomic counters                                    | Implemented with limitations | Atomic counters in non-compute shaders are not implemented yet. |
| Shader image load/store operations                 | Implemented with limitations | See notes [1] [below](#notes-1) |
| Shader storage buffer objects                      | Implemented with limitations | See notes [2] [below](#notes-2) |
| Shader helper invocation (shading language only)   | Hard to implement            | The equivalent of gl_HelperInvocation is WaveIsHelperLane which requres SM6. |
| Separate shader objects                            | Unimplemented                | It can be implemented with medium complexity. |
| Stencil texturing                                  | Unimplemented                | It can be implemented with medium complexity.  Refer [here](https://stackoverflow.com/questions/34601325/directx11-read-stencil-bit-from-compute-shader). |

### Notes [1]
* Images in non-compute shaders are not implemented yet.
* Multiple image variables are not allowed to be bound to the same image unit which refers to the same layer and level of a texture image. It means image aliasing is not supported.
* The same layer and level of a texture are not allowed to be bound to multiple image units.
* When a texture is bound to an image unit, the image unit format must exactly match the texture internal format. Similarly, the format layout qualifier for an image variable must exactly match the format of the image unit. Re-interpretation is not supported. See [here](http://anglebug.com/42261723).

### Notes [2]
* Shader storage blocks in non-compute shaders are not implemented yet.
* Multiple shader storage blocks are not allowed to be bound to the same buffer. See [here](http://anglebug.com/42261718).
