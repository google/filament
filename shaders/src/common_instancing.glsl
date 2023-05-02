//------------------------------------------------------------------------------
// Instancing
// ------------------------------------------------------------------------------------------------

PerRenderableData object_uniforms;

void initInstanceUniforms(out PerRenderableData p) {
    // We're copying each field separately to workaround an issue in some Adreno drivers
    // that fail on non-const array access in a UBO. Accessing the fields works however.
    // e.g.: this fails `p = objectUniforms.data[instance_index];`
    p.worldFromModelMatrix = objectUniforms.data[instance_index].worldFromModelMatrix;
    p.worldFromModelNormalMatrix = objectUniforms.data[instance_index].worldFromModelNormalMatrix;
    p.morphTargetCount = objectUniforms.data[instance_index].morphTargetCount;
    p.flagsChannels = objectUniforms.data[instance_index].flagsChannels;
    p.objectId = objectUniforms.data[instance_index].objectId;
    p.userData = objectUniforms.data[instance_index].userData;
}

void initObjectUniforms(out PerRenderableData p) {
#if defined(MATERIAL_HAS_INSTANCES)
    if ((objectUniforms.data[0].flagsChannels & FILAMENT_OBJECT_INSTANCE_BUFFER_BIT) != 0) {
        // the object has an instance buffer
        initInstanceUniforms(p);
        return;
    }

    // the material manages instancing, all instances share the same uniform block.
    p = objectUniforms.data[0];

#else
    // automatic instancing was used, each instance has its own uniform block.
    initInstanceUniforms(p);
#endif
}

//------------------------------------------------------------------------------
// Instance access
//------------------------------------------------------------------------------

#if defined(MATERIAL_HAS_INSTANCES)
/** @public-api */
int getInstanceIndex() {
    return instance_index;
}
#endif
