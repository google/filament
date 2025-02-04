//------------------------------------------------------------------------------
// Common Varyings
//------------------------------------------------------------------------------

// The `multiview_data` variable is to pass the current state of multiview set in the vertex shader
// to the fragment shader. `x` means whether multiview is used, `y` means the current view index.
// Neither the multiview feature nor the `flat` keyword is supported in FL0. So, for FL0, declare
// it as a non-flat and non-varying variable and set it to zeros as a placeholder.
#if MATERIAL_FEATURE_LEVEL > 0
LAYOUT_LOCATION(99) flat VARYING ivec2 multiview_data;
#else
ivec2 multiview_data = ivec2(0, 0);
#endif
