Randi Rost
Hewlett-Packard
970-229-2447
rost@tortola.fc.hp.com

-----------------------------------------------------------------------------


OpenGL RGBA Image Rendering Pipeline (destination is an RGBA visual)

1.  Image data exist in main memory (as either RGBA or color index values)
2.  Extract subimage, performing byte swapping if necessary
3.  If pixels are RGBA pixels, convert all components to floats (logically,
    but not always necessary in practice)
4.  If pixels are RGBA pixels, convert all component groups to full RGBA
    (again, logically, but not necessary in practice)
5a. If pixels are RGBA pixels, perform ax+b operation on each component
5b. If pixels are color index pixels, perform index shift and offset on
    each index value
6a. If pixels are RGBA values, go through lookup table (OpenGL RGBA to RGBA
    lookup, controlled by GL_MAP_COLOR)
6b. If pixels are color index values, go through lookup table (OpenGL color
    index to RGBA lookup, controlled by GL_MAP_COLOR)
7.  LUT (COLOR_TABLE_SGI from the SGI_color_table extension)
8.  Convolve (including post-convolution scale and bias)
9.  LUT (POST_CONVOLUTION_COLOR_TABLE_SGI, from SGI_color_table extension)
10. Color matrix (including post-color matrix scale and bias)
11. LUT (POST_COLOR_MATRIX_COLOR_TABLE_SGI, from SGI_color_table extension)
12. Histogram, min/max
13. Zoom
14. Write fragments to the display (includes normal per-fragment operations:
    texture, fog, blend, etc.)

Notes:

Steps #1-6 and #13-14 are in core OpenGL.  The rest come from the imaging
extensions. Steps #7, 9, 11 come from the SGI_color_table extension.
Step #8 comes from the EXT_convolution extension.  Step #10 comes from
the SGI_color_matrix extension.  Step #12 comes from the EXT_histogram
extension.

You may notice there is nothing to support image rotation.  I would propose
that we add an affine transformation stage after step #9 in order to support
image scale, rotate, and translate with proper resampling controls.  To follow
the example set for the convolve and color matrix extensions, we may want
to follow this with a LUT that would be the means for accomplishing
window-level mapping.

The zoom function defined by core OpenGL seems insufficient and occurs at the
wrong point in the pipeline.

How are the typical imaging scale/rotate/resample operations performed in
the SGI model?  Steps 1-9 are performed on an image that is being loaded into
texture memory.  A rectangle is drawn with the geometry, texture coordinates,
and texturing controls to provide the proper scaling/rotation/resampling
behavior.  This approach seems to have several drawbacks:

	1) The programming model is not simple
	2) On many architectures, traffic on the graphics bus may be
	   doubled (write to texture memory, followed by a read from
	   texture memory and write into the framebuffer)
	3) Requires texture mapping hardware to have reasonable performance

The attached extension specification proposes adding an image transformation
step that occurs immediately after step #9.  This image transformation step
supports image scale/rotate/translate operations, followed by proper
resampling, followed by another color lookup table that can be used for
window level mapping.

