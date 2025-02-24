==============
PostProcessing
==============

.. figure:: ./PostProcessing.png

This example demonstrates a set of heavily-optimised bloom post-processing implementations.

API
---
* Vulkan

Description
-----------
This example demonstrates a set of heavily-optimised bloom post-processing implementations including:

- Reference implementation of a separated Gaussian Blur.
- Linear sampler-optimised separated Gaussian Blur.
- Sliding average compute-based separated Gaussian Blur.
- Linear sampler-optimised separated Gaussian Blur with samples of negligible value truncated. This means the approximate blurs can be achieved with far fewer samples.
- Hybrid Gaussian Blur using the truncated separated Gaussian Blur along with a sliding average-based Gaussian Blur.
- Kawase Blur - Framebuffer post-processing effects in "DOUBLE-S.T.E.A.L." aka "Wreckless".
- Dual Filter - Bandwidth-efficient rendering - siggraph2015-mmg-marius.
- Tent Filter - Next generation post-processing in "Call Of Duty Advanced Warfare".

Other than the Dual Filter and Tent Filter, the bloom post-processing implementations follow a similar high-level pattern:

1. Downsampling the brighter regions of an input image to a lower resolution. 
2. Several post-process passes, each working from the output of the previous pass, rendering to intermediate textures. 
3. The resulting blurred image is then composited onto the original image to create a smooth bloom around the brighter regions.

Controls
--------
- Left/Right - Cycle through the various bloom implementations
- Up/Down - Increase/decrease the size of the bloom intensity
- Action1 - Pause
- Action2 - Enable/disable rendering of bloom only
- Quit - Close the application