// Adreno drivers seem to ignore precision qualifiers in structs, unless they're used in
// UBOs, which is is the case here.
struct ShadowData {
    highp mat4 lightFromWorldMatrix;
    highp vec3 direction;
    float normalBias;
    highp vec4 lightFromWorldZ;
    float texelSizeAtOneMeter;
    float bulbRadiusLs;
    float nearOverFarMinusNear;
    bool elvsm;
};

struct BoneData {
    highp mat3x4 transform;    // bone transform is mat4x3 stored in row-major (last row [0,0,0,1])
    highp uvec4 cof;           // 8 first cofactor matrix of transform's upper left
};

struct PerRenderableData {
    highp mat4 worldFromModelMatrix;
    highp mat3 worldFromModelNormalMatrix;
    highp uint morphTargetCount;
    highp uint flagsChannels;                   // see packFlags() below (0x00000fll)
    highp uint objectId;                        // used for picking
    highp float userData;   // TODO: We need a better solution, this currently holds the average local scale for the renderable
    highp vec4 reserved[8];
};
