/*
 * Screen-space reflection shading
 */
vec4 evaluateMaterial(const MaterialInputs material) {
#if defined(MATERIAL_HAS_REFLECTIONS)
    vec4 color = vec4(0.0);
    if (frameUniforms.ssrDistance > 0.0) {
#if defined(MATERIAL_HAS_ANISOTROPY)
#if defined(GEOMETRIC_SPECULAR_AA)
        float kernelRoughness = computeKernelRoughness(getWorldGeometricNormalVector());
#else
        float kernelRoughness = 0.0;
#endif
        PixelParams pixel;
        getRoughnessPixelParams(material, pixel, kernelRoughness);
        getAnisotropyPixelParams(material, pixel);
        vec3 r = getReflectedVector(pixel, shading_view, shading_normal);
#else
        vec3 r = shading_reflected;
#endif

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
