//------------------------------------------------------------------------------
// Common Varyings
//------------------------------------------------------------------------------

// The `filament_multiview_data` variable is to pass the current state of multiview set in the
// vertex shader to the fragment shader. `x` means whether multiview is used, `y` means the current
// view index. Neither the multiview feature nor the `flat` keyword is supported in FL0. So, for
// FL0, declare it as a non-flat and non-varying variable and set it to zeros as a placeholder.
// `MATERIAL_FEATURE_LEVEL > 0` is used as a workaround instead of `defined(VARIANT_HAS_STEREO)` to
// make this line available for post-processing where we cannot use VARIANT_HAS_STEREO. The
// downside of this approach is that `flat VARYING` is used for non-STE shaders as well.
#if MATERIAL_FEATURE_LEVEL > 0 && defined(FILAMENT_STEREO_MULTIVIEW)
LAYOUT_LOCATION(12) flat VARYING ivec2 filament_multiview_data;
#else
ivec2 filament_multiview_data = ivec2(0, 0);
#endif
