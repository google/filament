#if defined(VARIANT_HAS_SHADOWING)
// Adreno drivers seem to ignore precision qualifiers in structs, unless they're used in
// UBOs, which is the case here.
struct ShadowData {
    highp mat4 lightFromWorldMatrix;
    highp vec4 lightFromWorldZ;
    highp vec4 scissorNormalized;
    mediump float texelSizeAtOneMeter;
    mediump float bulbRadiusLs;
    mediump float nearOverFarMinusNear;
    mediump float normalBias;
    bool elvsm;
    mediump uint layer;
    mediump uint reserved1;
    mediump uint reserved2;
};
#endif

#if defined(VARIANT_HAS_SKINNING_OR_MORPHING)
struct BoneData {
    highp mat3x4 transform;    // bone transform is mat4x3 stored in row-major (last row [0,0,0,1])
    highp float3 cof0;         // 3 first cofactor matrix of transform's upper left
    highp float cof1x;         // 4th cofactor
};
#endif

struct PerRenderableData {
    highp mat4 worldFromModelMatrix;
    highp mat3 worldFromModelNormalMatrix;
    highp int morphTargetCount;
    highp int flagsChannels;                   // see packFlags() below (0x00000fll)
    highp int objectId;                        // used for picking
    highp float userData;   // TODO: We need a better solution, this currently holds the average local scale for the renderable
    highp vec4 reserved[8];
};

// Bits for flagsChannels
#define FILAMENT_OBJECT_SKINNING_ENABLED_BIT   0x100
#define FILAMENT_OBJECT_MORPHING_ENABLED_BIT   0x200
#define FILAMENT_OBJECT_CONTACT_SHADOWS_BIT    0x400
#define FILAMENT_OBJECT_INSTANCE_BUFFER_BIT    0x800
