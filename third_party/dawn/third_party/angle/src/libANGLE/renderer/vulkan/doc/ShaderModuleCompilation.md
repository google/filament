# Shader Module Compilation

ANGLE converts application shaders into Vulkan [VkShaderModules][VkShaderModule] through a series
of steps:

1. **ANGLE Internal Translation**: The initial calls to `glCompileShader` are passed to the [ANGLE
shader translator][translator]. The translator compiles application shaders into SPIR-V, after
transforming GLSL to conform to the [GL_KHR_vulkan_glsl][GL_KHR_vulkan_glsl] extension spec with
some additional workarounds and emulation. We emulate OpenGL's different depth range, viewport y
flipping, default uniforms, and emulate a number of extensions among others. For more info see
[TranslatorVulkan.cpp][TranslatorVulkan.cpp]. After initial compilation, the SPIR-V blobs are not
complete. The translator initially assigns resources and in/out variables arbitrary descriptor set,
binding and location indices. The correct values are determined at link time. For the sake of
transform feedback, some additional code is generated to be removed or modified during SPIR-V
transformation.

   The translator outputs some feature code conditional to Vulkan specialization constants, which are
resolved at draw-time. For example, for emulating Dithering and Android surface rotation.

1. **Link Time**: During a call to `glLinkProgram` the Vulkan back-end can know the necessary
locations and properties to write to connect the shader stage interfaces. However, some draw-time
information is still necessary to finalize the SPIR-V; for example whether "early fragment tests"
can be enabled. As an optimization, the SPIR-V is transformed with arbitrary settings and a Vulkan
pipeline object is created in an attempt to warm the Vulkan pipeline cache.

1. **Draw-Time SPIR-V Transformation and Pipeline Creation**: Once the application records a draw
call, a transformation pass is done on the generated SPIR-V to update the arbitrary descriptor set,
binding and location indices set in step 1. Additionally, component and various transform feedback
decorations are added, inactive varyings are removed, early fragment tests are enabled or disabled,
debug info is removed and pre-rotation is applied. At this time, `VkShaderModule`s are created (and
cached). The appropriate specialization constants are then resolved and the `VkPipeline` object is
created (see details [here](PipelineCreation)).  Note that we currently don't use
[SPIRV-Tools][SPIRV-Tools] to perform any SPIR-V optimization. This could be something to improve on
in the future.

See the below diagram for a high-level view of the shader translation flow:

<!-- Generated from https://bramp.github.io/js-sequence-diagrams/
     Note: remove whitespace in - -> arrows.
participant App
participant "ANGLE Front-end"
participant "Vulkan Back-end"
participant "ANGLE Translator"
participant "Link-Time SPIR-V Transformer"

App->"ANGLE Front-end": glCompileShader (VS)
"ANGLE Front-end"->"Vulkan Back-end": ShaderVk::compile
"Vulkan Back-end"->"ANGLE Translator": sh::Compile
"ANGLE Translator"- ->"ANGLE Front-end": return SPIR-V

Note right of "ANGLE Front-end": SPIR-V is using bogus\ndecorations to be\ncorrected at link time.

Note right of App: Same for FS, GS, etc...

App->"ANGLE Front-end": glCreateProgram (...)
App->"ANGLE Front-end": glAttachShader (...)
App->"ANGLE Front-end": glLinkProgram
"ANGLE Front-end"->"Vulkan Back-end": ProgramVk::link

Note right of "Vulkan Back-end": ProgramVk inits uniforms,\nlayouts, and descriptors.

"Vulkan Back-end"->"Link-Time SPIR-V Transformer": SpvGetShaderSpirvCode
"Link-Time SPIR-V Transformer"- ->"Vulkan Back-end": retrieve SPIR-V and determine decorations
"Vulkan Back-end"- ->"ANGLE Front-end": return success

Note right of App: App execution continues...

App->"ANGLE Front-end": glDrawArrays (any draw)
"ANGLE Front-end"->"Vulkan Back-end": ContextVk::drawArrays

"Vulkan Back-end"->"Link-Time SPIR-V Transformer": SpvTransformSpirvCode
"Link-Time SPIR-V Transformer"- ->"Vulkan Back-end": return transformed SPIR-V

Note right of "Vulkan Back-end": We init VkShaderModules\nand VkPipeline then\nrecord the draw.

"Vulkan Back-end"- ->"ANGLE Front-end": return success
-->

![Vulkan Shader Translation Flow](https://raw.githubusercontent.com/google/angle/main/src/libANGLE/renderer/vulkan/doc/img/VulkanShaderTranslation.svg?sanitize=true)

[GL_KHR_vulkan_glsl]: https://github.com/KhronosGroup/GLSL/blob/main/extensions/khr/GL_KHR_vulkan_glsl.txt
[SPIRV-Tools]: https://github.com/KhronosGroup/SPIRV-Tools
[translator]: https://chromium.googlesource.com/angle/angle/+/refs/heads/main/src/compiler/translator/
[TranslatorVulkan.cpp]: https://chromium.googlesource.com/angle/angle/+/refs/heads/main/src/compiler/translator/TranslatorVulkan.cpp
[VkShaderModule]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkShaderModule.html
[PipelineCreation]: PipelineCreation.md
