//------------------------------------------------------------------------------
// Fog
//------------------------------------------------------------------------------

vec3 fog(vec3 color, vec3 cameraToWorld, vec3 wsCameraPosition, vec3 up) {
    if (frameUniforms.fogDensity > 0.0) {
        float wsCameraHeight = dot(up, wsCameraPosition);

        // clamp to camera far plane so the skybox participates to the fog
        // NOTE: the double-min is needed here, or some compilers won't work, apparently, it's
        // because cameraToWorld may have infinites, min() with a non-constant, just doesn't work.
        float cameraToWorldDistance = min(frameUniforms.cameraFar, min(MEDIUMP_FLT_MAX, length(cameraToWorld)));

        // fog density with respect to camera's altitude
        float density = frameUniforms.fogDensity * exp2(-frameUniforms.fogHeightFalloff * (wsCameraHeight - frameUniforms.fogHeight));

        // falloff due to height of the shaded fragment w.r.t. the camera
        float h = dot(cameraToWorld, up);
        h = abs(h) > 0.01 ? h : 0.01;
        float falloff = max(frameUniforms.fogHeightFalloff * h, -127.0);

        // fog density at shaded fragment w.r.t. camera's altitude
        float densityFalloff = density * (1.0 - exp2(-falloff)) / falloff;

        // take fog start distance into account
        float lineIntegral = densityFalloff * max(cameraToWorldDistance - frameUniforms.fogStart, 0.0);
        float fogFactor = max(clamp(exp2(-lineIntegral), 0.0, 1.0), frameUniforms.fogMinOpacity);

        vec3 foggedColor = frameUniforms.fogColor * (1.0 - fogFactor);
        if (frameUniforms.fogInscatteringSize > 0.0) {
            vec3 lightDirectionNorm = -normalize(frameUniforms.lightDirection);
            vec3 cameraToWorldUnit = cameraToWorld * (1.0 / cameraToWorldDistance);
            float lightInscatteringFactor = max(dot(cameraToWorldUnit, lightDirectionNorm), 0.0);
            vec3 lightInscattering = frameUniforms.lightColorIntensity.rgb * frameUniforms.lightColorIntensity.w * pow(lightInscatteringFactor, frameUniforms.fogInscatteringSize);
            float lightInscatteringIntegral = densityFalloff * max(cameraToWorldDistance - frameUniforms.fogInscatteringStart, 0.0);
            float inscatteringFactor = clamp(exp2(-lightInscatteringIntegral), 0.0, 1.0);
            foggedColor += lightInscattering * (1.0 - inscatteringFactor);
        }

        return color * fogFactor + foggedColor;
    }
    return color;
}
