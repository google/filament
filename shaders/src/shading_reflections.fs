/*
 * screen-space reflection shading
 */
vec4 evaluateMaterial(const MaterialInputs material) {

#if defined(MATERIAL_HAS_REFLECTIONS)
    PixelParams pixel;
    getAnisotropyPixelParams(material, pixel);
    vec4 color = vec4(0.0);
    if (frameUniforms.ssrDistance > 0.0) {
        vec3 r = getReflectedVector(pixel, shading_view, shading_normal);
        // evaluateScreenSpaceReflections will set the value of color if there's a hit.
        // color.a contains the reflection's contribution.
        color = evaluateScreenSpaceReflections(r);
    }
#else
    // objects without reflections just erase the framebuffer, they need to be drawn in the
    // reflection buffer because they could be partially occluding other objects with reflections.
    const vec4 color = vec4(0.0);
#endif

    return color;
}
