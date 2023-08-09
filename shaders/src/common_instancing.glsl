//------------------------------------------------------------------------------
// Instancing
// ------------------------------------------------------------------------------------------------

highp mat4 object_uniforms_worldFromModelMatrix;
highp mat3 object_uniforms_worldFromModelNormalMatrix;
highp int object_uniforms_morphTargetCount;
highp int object_uniforms_flagsChannels;                   // see packFlags() below (0x00000fll)
highp int object_uniforms_objectId;                        // used for picking
highp float object_uniforms_userData;   // TODO: We need a better solution, this currently holds the average local scale for the renderable

void initObjectUniforms() {
    // Adreno drivers workarounds:
    // - We need to copy each field separately because non-const array access in a UBO fails
    //    e.g.: this fails `p = objectUniforms.data[instance_index];`
    // - We can't use a struct to hold the result because Adreno driver ignore precision qualifiers
    //   on fields of structs, unless they're in a UBO (which we just copied out of).

#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
#if defined(MATERIAL_HAS_INSTANCES)
    if ((objectUniforms.data[0].flagsChannels & FILAMENT_OBJECT_INSTANCE_BUFFER_BIT) == 0) {
        // the material manages instancing, all instances share the same uniform block.
        object_uniforms_worldFromModelMatrix        = objectUniforms.data[0].worldFromModelMatrix;
        object_uniforms_worldFromModelNormalMatrix  = objectUniforms.data[0].worldFromModelNormalMatrix;
        object_uniforms_morphTargetCount            = objectUniforms.data[0].morphTargetCount;
        object_uniforms_flagsChannels               = objectUniforms.data[0].flagsChannels;
        object_uniforms_objectId                    = objectUniforms.data[0].objectId;
        object_uniforms_userData                    = objectUniforms.data[0].userData;
    } else
#endif
    {
        highp int index = instance_index >> (frameUniforms.instanceIndexShift & 0xFu);
        // the object has an instance buffer
        object_uniforms_worldFromModelMatrix        = objectUniforms.data[index].worldFromModelMatrix;
        object_uniforms_worldFromModelNormalMatrix  = objectUniforms.data[index].worldFromModelNormalMatrix;
        object_uniforms_morphTargetCount            = objectUniforms.data[index].morphTargetCount;
        object_uniforms_flagsChannels               = objectUniforms.data[index].flagsChannels;
        object_uniforms_objectId                    = objectUniforms.data[index].objectId;
        object_uniforms_userData                    = objectUniforms.data[index].userData;
    }
#else
    object_uniforms_worldFromModelMatrix        = objectUniforms.data[0].worldFromModelMatrix;
    object_uniforms_worldFromModelNormalMatrix  = objectUniforms.data[0].worldFromModelNormalMatrix;
    object_uniforms_morphTargetCount            = objectUniforms.data[0].morphTargetCount;
    object_uniforms_flagsChannels               = objectUniforms.data[0].flagsChannels;
    object_uniforms_objectId                    = objectUniforms.data[0].objectId;
    object_uniforms_userData                    = objectUniforms.data[0].userData;
#endif
}

//------------------------------------------------------------------------------
// Instance access
//------------------------------------------------------------------------------

#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
#if defined(MATERIAL_HAS_INSTANCES)
/** @public-api */
int getInstanceIndex() {
    return instance_index;
}
#endif
#endif
