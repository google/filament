==============================
ImageBasedLightingMapGenerator
==============================

A command-line tool showing how to use headless Vulkan to generate Image Based Lighting irradiance and reflection maps from a skybox.

API
---
* Vulkan
 
Description
-----------
This tool is different from the rest of the SDK in that it is a command line tool that can be used to generate two maps used in Image Based Lighting using GPU shaders to optimise the procedure. A single-thread cpu calculation would easily take minutes for each of the maps, hence this Vulkan implementation.
The input is a skybox, a Cubemap that represents the entire environment around a point in space. 
The Irradiance map the uses this skybox to calculate the incoming energy for each direction, in other words how much does this skybox light a surface with a specific orientation, assuming perfect diffusion.
The Pre-Filtered Reflection map is essentially a copy of the skybox whose mip-maps are mapped to different roughness values, with smaller mipmaps being progressively more blurred in order to be used in larger roughness values, simulating the effect of rougher surfaces having blurrier reflections.

Controls
--------
- Usage: VulkanIBLMapGenerator.exe [input cubemap] [options]
- -skipDiffuse                  do not create a diffuse irradiance map
- -diffuseSize=[NUMBER]         the size of the output Irradiance map
- -diffuseSamples=[NUMBER]      the number of samples per output texel of the irradiance map (default 10000)
- -skipSpecular                 do not create a Pre-Filtered Reflection map
- -specularSize=[NUMBER]        the size of the output Pre-Filtered Reflection map
- -specularSamples=[NUMBER]     the number of samples per output texel of the irradiance map (default 10000)
- -specularDiscardMips=[NUMBER] the number of specular mips NOT to generate (i.e. 2 means that the smallest mipmap will be 4x4)
