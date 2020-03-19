//------------------------------------------------------------------------------
// Fog
//------------------------------------------------------------------------------

vec3 fog(vec3 color, vec3 view, vec3 wsCameraPosition, vec3 up) {
    if (frameUniforms.fogDensity > 0.0) {
        float wsCameraHeight = dot(up, wsCameraPosition);

        // v can have infinites (usually due to the skybox)
        float cameraToWorldDistance = min(MEDIUMP_FLT_MAX, length(view));

        // clamp to camera far plane so the skybox participates to the fog
        float cameraToWorldDistanceClamped = min(frameUniforms.cameraFar, cameraToWorldDistance);

        // fog density with respect to camera's altitude
        float density = frameUniforms.fogDensity * exp2(-frameUniforms.fogHeightFalloff * (wsCameraHeight - frameUniforms.fogHeight));

        // falloff due to height of the shaded fragment w.r.t. the camera
        float h = dot(view, up);
        h = abs(h) > 0.01 ? h : 0.01;
        float falloff = max(frameUniforms.fogHeightFalloff * h, -127.0);

        // fog density at shaded fragment w.r.t. camera's altitude
        float densityFalloff = density * (1.0 - exp2(-falloff)) / falloff;

        // take fog start distance into account
        float lineIntegral = densityFalloff * max(cameraToWorldDistanceClamped - frameUniforms.fogStart, 0.0);
        float fogFactor = max(clamp(exp2(-lineIntegral), 0.0, 1.0), 1.0 - frameUniforms.fogMaxOpacity);

        vec3 foggedColor = frameUniforms.fogColor * (1.0 - fogFactor);
        if (frameUniforms.fogInscatteringSize > 0.0) {
            vec3 lightDirectionNorm = -normalize(frameUniforms.lightDirection);
            vec3 cameraToWorldUnit = view * (1.0 / cameraToWorldDistance);
            float lightInscatteringFactor = max(dot(cameraToWorldUnit, lightDirectionNorm), 0.0);
            vec3 lightInscattering = frameUniforms.lightColorIntensity.rgb * frameUniforms.lightColorIntensity.w * pow(lightInscatteringFactor, frameUniforms.fogInscatteringSize);
            float lightInscatteringIntegral = densityFalloff * max(cameraToWorldDistanceClamped - frameUniforms.fogInscatteringStart, 0.0);
            float inscatteringFactor = clamp(exp2(-lightInscatteringIntegral), 0.0, 1.0);
            foggedColor += lightInscattering * (1.0 - inscatteringFactor);
        }

        return color * fogFactor + foggedColor;
    }
    return color;
}
