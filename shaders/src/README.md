# Shader Code Directory

This directory contains GLSL (OpenGL Shading Language) code snippets, organized by their function and stage in the graphics pipeline. These snippets are designed to be used as **building blocks** by the **Shader Generator** within this project. Instead of being compiled directly, these snippets are combined and processed to generate complete, functional shader programs for various purposes.


## File Naming Convention

Shader files within this directory follow a consistent naming convention to easily identify their purpose and usage:

`prefix_name.suffix`

### Prefixes

The prefix indicates the domain or category of the shader code:

*   **`surface`:** Snippets with this prefix are intended for use in **surface shaders**. Surface shaders define the visual properties of object surfaces, handling aspects like lighting, texturing, and material properties.
*   **`post_process`:** Snippets with this prefix are used in **post-processing shaders**. Post-processing shaders operate on the entire rendered image after the main rendering pass to achieve effects such as blur, bloom, color correction, and more.
*   **`common`:** Snippets with this prefix contain **shared code** that can be used by **both** `surface` and `post_process` shaders. This includes utility functions, common data structures, and reusable logic, promoting modularity and reducing code duplication.
*   **`inline`:** Snippets with this prefix are **specifically designed to be directly included within other shader files** during the generation process. They often contain reusable functions or macros that need to be present in the final shader code but not necessarily categorized as `surface` or `post_process`.

### Suffixes (Extensions)

The suffix (file extension) specifies the **shader stage** that the code snippet is intended for:

*   **`.vs`:**  Files with this extension contain code snippets for **vertex shaders**. Vertex shaders operate on individual vertices and are typically used to transform vertex positions, calculate normals, and pass data to the fragment shader.
*   **`.fs`:** Files with this extension contain code snippets for **fragment shaders**. Fragment shaders, also known as pixel shaders, determine the final color of each pixel. They are used for lighting, texturing, and other per-pixel operations.
*   **`.cs`:** Files with this extension contain code snippets for **compute shaders**. Compute shaders are used for general-purpose computation on the GPU, outside the traditional rendering pipeline. They can be used for tasks like physics simulations, image processing, and more. (Currently not implemented yet)
*   **`.glsl`:** Files with this extension contain **reusable GLSL code snippets, including functions, structures, or constants**, that are intended to be included or linked into multiple shader stages (vertex, fragment, or compute). This is a generic extension to signify that the file does not belong exclusively to a single shader stage.

## Example

*   `surface_light_directional.fs`: Fragment shader snippets for objects using the directional lighting model.
*   `post_process_getters.vs`: Vertex shader snippets containing reusable getter functions for post-processing shaders.
*   `common_math.glsl`: GLSL code with reusable math constants and functions shared by both surface and post-processing shaders.
