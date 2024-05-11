//------------------------------------------------------------------------------
// Instancing
// ------------------------------------------------------------------------------------------------

highp mat4 object_uniforms_worldFromModelMatrix;
highp mat3 object_uniforms_worldFromModelNormalMatrix;
highp int object_uniforms_morphTargetCount;
highp int object_uniforms_flagsChannels;                   // see packFlags() below (0x00000fll)
highp int object_uniforms_objectId;                        // used for picking
highp float object_uniforms_userData;   // TODO: We need a better solution, this currently holds the average local scale for the renderable

//------------------------------------------------------------------------------
// Instance access
//------------------------------------------------------------------------------

void initObjectUniforms() {
    // Adreno drivers workarounds:
    // - We need to copy each field separately because non-const array access in a UBO fails
    //    e.g.: this fails `p = objectUniforms.data[instance_index];`
    // - We can't use a struct to hold the result because Adreno driver ignore precision qualifiers
    //   on fields of structs, unless they're in a UBO (which we just copied out of).

#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
    highp int i;
#   if defined(MATERIAL_HAS_INSTANCES)
    // instancing handled by the material
    if ((objectUniforms.data[0].flagsChannels & FILAMENT_OBJECT_INSTANCE_BUFFER_BIT) != 0) {
        // hybrid instancing, we have a instance buffer per object
        i = logical_instance_index;
    } else {
        // fully manual instancing
        i = 0;
    }
#   else
    // automatic instancing
    i = logical_instance_index;
#   endif
#else
    // we don't support instancing (e.g. ES2)
    const int i = 0;
#endif
    object_uniforms_worldFromModelMatrix        = objectUniforms.data[i].worldFromModelMatrix;
    object_uniforms_worldFromModelNormalMatrix  = objectUniforms.data[i].worldFromModelNormalMatrix;
    object_uniforms_morphTargetCount            = objectUniforms.data[i].morphTargetCount;
    object_uniforms_flagsChannels               = objectUniforms.data[i].flagsChannels;
    object_uniforms_objectId                    = objectUniforms.data[i].objectId;
    object_uniforms_userData                    = objectUniforms.data[i].userData;
}

#if defined(FILAMENT_HAS_FEATURE_INSTANCING) && defined(MATERIAL_HAS_INSTANCES)
/** @public-api */
highp int getInstanceIndex() {
    return logical_instance_index;
}
#endif
