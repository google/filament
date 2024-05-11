#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_View
{
    float4x4 View_TranslatedWorldToClip;
    float4x4 View_WorldToClip;
    float4x4 View_TranslatedWorldToView;
    float4x4 View_ViewToTranslatedWorld;
    float4x4 View_TranslatedWorldToCameraView;
    float4x4 View_CameraViewToTranslatedWorld;
    float4x4 View_ViewToClip;
    float4x4 View_ViewToClipNoAA;
    float4x4 View_ClipToView;
    float4x4 View_ClipToTranslatedWorld;
    float4x4 View_SVPositionToTranslatedWorld;
    float4x4 View_ScreenToWorld;
    float4x4 View_ScreenToTranslatedWorld;
    packed_float3 View_ViewForward;
    float PrePadding_View_844;
    packed_float3 View_ViewUp;
    float PrePadding_View_860;
    packed_float3 View_ViewRight;
    float PrePadding_View_876;
    packed_float3 View_HMDViewNoRollUp;
    float PrePadding_View_892;
    packed_float3 View_HMDViewNoRollRight;
    float PrePadding_View_908;
    float4 View_InvDeviceZToWorldZTransform;
    float4 View_ScreenPositionScaleBias;
    packed_float3 View_WorldCameraOrigin;
    float PrePadding_View_956;
    packed_float3 View_TranslatedWorldCameraOrigin;
    float PrePadding_View_972;
    packed_float3 View_WorldViewOrigin;
    float PrePadding_View_988;
    packed_float3 View_PreViewTranslation;
    float PrePadding_View_1004;
    float4x4 View_PrevProjection;
    float4x4 View_PrevViewProj;
    float4x4 View_PrevViewRotationProj;
    float4x4 View_PrevViewToClip;
    float4x4 View_PrevClipToView;
    float4x4 View_PrevTranslatedWorldToClip;
    float4x4 View_PrevTranslatedWorldToView;
    float4x4 View_PrevViewToTranslatedWorld;
    float4x4 View_PrevTranslatedWorldToCameraView;
    float4x4 View_PrevCameraViewToTranslatedWorld;
    packed_float3 View_PrevWorldCameraOrigin;
    float PrePadding_View_1660;
    packed_float3 View_PrevWorldViewOrigin;
    float PrePadding_View_1676;
    packed_float3 View_PrevPreViewTranslation;
    float PrePadding_View_1692;
    float4x4 View_PrevInvViewProj;
    float4x4 View_PrevScreenToTranslatedWorld;
    float4x4 View_ClipToPrevClip;
    float4 View_TemporalAAJitter;
    float4 View_GlobalClippingPlane;
    float2 View_FieldOfViewWideAngles;
    float2 View_PrevFieldOfViewWideAngles;
    float4 View_ViewRectMin;
    float4 View_ViewSizeAndInvSize;
    float4 View_BufferSizeAndInvSize;
    float4 View_BufferBilinearUVMinMax;
    int View_NumSceneColorMSAASamples;
    float View_PreExposure;
    float View_OneOverPreExposure;
    float PrePadding_View_2012;
    float4 View_DiffuseOverrideParameter;
    float4 View_SpecularOverrideParameter;
    float4 View_NormalOverrideParameter;
    float2 View_RoughnessOverrideParameter;
    float View_PrevFrameGameTime;
    float View_PrevFrameRealTime;
    float View_OutOfBoundsMask;
    float PrePadding_View_2084;
    float PrePadding_View_2088;
    float PrePadding_View_2092;
    packed_float3 View_WorldCameraMovementSinceLastFrame;
    float View_CullingSign;
    float View_NearPlane;
    float View_AdaptiveTessellationFactor;
    float View_GameTime;
    float View_RealTime;
    float View_DeltaTime;
    float View_MaterialTextureMipBias;
    float View_MaterialTextureDerivativeMultiply;
    uint View_Random;
    uint View_FrameNumber;
    uint View_StateFrameIndexMod8;
    uint View_StateFrameIndex;
    float View_CameraCut;
    float View_UnlitViewmodeMask;
    float PrePadding_View_2164;
    float PrePadding_View_2168;
    float PrePadding_View_2172;
    float4 View_DirectionalLightColor;
    packed_float3 View_DirectionalLightDirection;
    float PrePadding_View_2204;
    float4 View_TranslucencyLightingVolumeMin[2];
    float4 View_TranslucencyLightingVolumeInvSize[2];
    float4 View_TemporalAAParams;
    float4 View_CircleDOFParams;
    float View_DepthOfFieldSensorWidth;
    float View_DepthOfFieldFocalDistance;
    float View_DepthOfFieldScale;
    float View_DepthOfFieldFocalLength;
    float View_DepthOfFieldFocalRegion;
    float View_DepthOfFieldNearTransitionRegion;
    float View_DepthOfFieldFarTransitionRegion;
    float View_MotionBlurNormalizedToPixel;
    float View_bSubsurfacePostprocessEnabled;
    float View_GeneralPurposeTweak;
    float View_DemosaicVposOffset;
    float PrePadding_View_2348;
    packed_float3 View_IndirectLightingColorScale;
    float View_HDR32bppEncodingMode;
    packed_float3 View_AtmosphericFogSunDirection;
    float View_AtmosphericFogSunPower;
    float View_AtmosphericFogPower;
    float View_AtmosphericFogDensityScale;
    float View_AtmosphericFogDensityOffset;
    float View_AtmosphericFogGroundOffset;
    float View_AtmosphericFogDistanceScale;
    float View_AtmosphericFogAltitudeScale;
    float View_AtmosphericFogHeightScaleRayleigh;
    float View_AtmosphericFogStartDistance;
    float View_AtmosphericFogDistanceOffset;
    float View_AtmosphericFogSunDiscScale;
    uint View_AtmosphericFogRenderMask;
    uint View_AtmosphericFogInscatterAltitudeSampleNum;
    float4 View_AtmosphericFogSunColor;
    packed_float3 View_NormalCurvatureToRoughnessScaleBias;
    float View_RenderingReflectionCaptureMask;
    float4 View_AmbientCubemapTint;
    float View_AmbientCubemapIntensity;
    float View_SkyLightParameters;
    float PrePadding_View_2488;
    float PrePadding_View_2492;
    float4 View_SkyLightColor;
    float4 View_SkyIrradianceEnvironmentMap[7];
    float View_MobilePreviewMode;
    float View_HMDEyePaddingOffset;
    float View_ReflectionCubemapMaxMip;
    float View_ShowDecalsMask;
    uint View_DistanceFieldAOSpecularOcclusionMode;
    float View_IndirectCapsuleSelfShadowingIntensity;
    float PrePadding_View_2648;
    float PrePadding_View_2652;
    packed_float3 View_ReflectionEnvironmentRoughnessMixingScaleBiasAndLargestWeight;
    int View_StereoPassIndex;
    float4 View_GlobalVolumeCenterAndExtent[4];
    float4 View_GlobalVolumeWorldToUVAddAndMul[4];
    float View_GlobalVolumeDimension;
    float View_GlobalVolumeTexelSize;
    float View_MaxGlobalDistance;
    float View_bCheckerboardSubsurfaceProfileRendering;
    packed_float3 View_VolumetricFogInvGridSize;
    float PrePadding_View_2828;
    packed_float3 View_VolumetricFogGridZParams;
    float PrePadding_View_2844;
    float2 View_VolumetricFogSVPosToVolumeUV;
    float View_VolumetricFogMaxDistance;
    float PrePadding_View_2860;
    packed_float3 View_VolumetricLightmapWorldToUVScale;
    float PrePadding_View_2876;
    packed_float3 View_VolumetricLightmapWorldToUVAdd;
    float PrePadding_View_2892;
    packed_float3 View_VolumetricLightmapIndirectionTextureSize;
    float View_VolumetricLightmapBrickSize;
    packed_float3 View_VolumetricLightmapBrickTexelSize;
    float View_StereoIPD;
    float View_IndirectLightingCacheShowFlag;
    float View_EyeToPixelSpreadAngle;
};

struct type_StructuredBuffer_v4float
{
    float4 _m0[1];
};

struct type_TranslucentBasePass
{
    uint TranslucentBasePass_Shared_Forward_NumLocalLights;
    uint TranslucentBasePass_Shared_Forward_NumReflectionCaptures;
    uint TranslucentBasePass_Shared_Forward_HasDirectionalLight;
    uint TranslucentBasePass_Shared_Forward_NumGridCells;
    packed_int3 TranslucentBasePass_Shared_Forward_CulledGridSize;
    uint TranslucentBasePass_Shared_Forward_MaxCulledLightsPerCell;
    uint TranslucentBasePass_Shared_Forward_LightGridPixelSizeShift;
    uint PrePadding_TranslucentBasePass_Shared_Forward_36;
    uint PrePadding_TranslucentBasePass_Shared_Forward_40;
    uint PrePadding_TranslucentBasePass_Shared_Forward_44;
    packed_float3 TranslucentBasePass_Shared_Forward_LightGridZParams;
    float PrePadding_TranslucentBasePass_Shared_Forward_60;
    packed_float3 TranslucentBasePass_Shared_Forward_DirectionalLightDirection;
    float PrePadding_TranslucentBasePass_Shared_Forward_76;
    packed_float3 TranslucentBasePass_Shared_Forward_DirectionalLightColor;
    float TranslucentBasePass_Shared_Forward_DirectionalLightVolumetricScatteringIntensity;
    uint TranslucentBasePass_Shared_Forward_DirectionalLightShadowMapChannelMask;
    uint PrePadding_TranslucentBasePass_Shared_Forward_100;
    float2 TranslucentBasePass_Shared_Forward_DirectionalLightDistanceFadeMAD;
    uint TranslucentBasePass_Shared_Forward_NumDirectionalLightCascades;
    uint PrePadding_TranslucentBasePass_Shared_Forward_116;
    uint PrePadding_TranslucentBasePass_Shared_Forward_120;
    uint PrePadding_TranslucentBasePass_Shared_Forward_124;
    float4 TranslucentBasePass_Shared_Forward_CascadeEndDepths;
    float4x4 TranslucentBasePass_Shared_Forward_DirectionalLightWorldToShadowMatrix[4];
    float4 TranslucentBasePass_Shared_Forward_DirectionalLightShadowmapMinMax[4];
    float4 TranslucentBasePass_Shared_Forward_DirectionalLightShadowmapAtlasBufferSize;
    float TranslucentBasePass_Shared_Forward_DirectionalLightDepthBias;
    uint TranslucentBasePass_Shared_Forward_DirectionalLightUseStaticShadowing;
    uint PrePadding_TranslucentBasePass_Shared_Forward_488;
    uint PrePadding_TranslucentBasePass_Shared_Forward_492;
    float4 TranslucentBasePass_Shared_Forward_DirectionalLightStaticShadowBufferSize;
    float4x4 TranslucentBasePass_Shared_Forward_DirectionalLightWorldToStaticShadow;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_576;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_580;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_584;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_588;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_592;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_596;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_600;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_604;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_608;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_612;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_616;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_620;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_624;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_628;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_632;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_636;
    uint TranslucentBasePass_Shared_ForwardISR_NumLocalLights;
    uint TranslucentBasePass_Shared_ForwardISR_NumReflectionCaptures;
    uint TranslucentBasePass_Shared_ForwardISR_HasDirectionalLight;
    uint TranslucentBasePass_Shared_ForwardISR_NumGridCells;
    packed_int3 TranslucentBasePass_Shared_ForwardISR_CulledGridSize;
    uint TranslucentBasePass_Shared_ForwardISR_MaxCulledLightsPerCell;
    uint TranslucentBasePass_Shared_ForwardISR_LightGridPixelSizeShift;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_676;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_680;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_684;
    packed_float3 TranslucentBasePass_Shared_ForwardISR_LightGridZParams;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_700;
    packed_float3 TranslucentBasePass_Shared_ForwardISR_DirectionalLightDirection;
    float PrePadding_TranslucentBasePass_Shared_ForwardISR_716;
    packed_float3 TranslucentBasePass_Shared_ForwardISR_DirectionalLightColor;
    float TranslucentBasePass_Shared_ForwardISR_DirectionalLightVolumetricScatteringIntensity;
    uint TranslucentBasePass_Shared_ForwardISR_DirectionalLightShadowMapChannelMask;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_740;
    float2 TranslucentBasePass_Shared_ForwardISR_DirectionalLightDistanceFadeMAD;
    uint TranslucentBasePass_Shared_ForwardISR_NumDirectionalLightCascades;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_756;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_760;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_764;
    float4 TranslucentBasePass_Shared_ForwardISR_CascadeEndDepths;
    float4x4 TranslucentBasePass_Shared_ForwardISR_DirectionalLightWorldToShadowMatrix[4];
    float4 TranslucentBasePass_Shared_ForwardISR_DirectionalLightShadowmapMinMax[4];
    float4 TranslucentBasePass_Shared_ForwardISR_DirectionalLightShadowmapAtlasBufferSize;
    float TranslucentBasePass_Shared_ForwardISR_DirectionalLightDepthBias;
    uint TranslucentBasePass_Shared_ForwardISR_DirectionalLightUseStaticShadowing;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_1128;
    uint PrePadding_TranslucentBasePass_Shared_ForwardISR_1132;
    float4 TranslucentBasePass_Shared_ForwardISR_DirectionalLightStaticShadowBufferSize;
    float4x4 TranslucentBasePass_Shared_ForwardISR_DirectionalLightWorldToStaticShadow;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1216;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1220;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1224;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1228;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1232;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1236;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1240;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1244;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1248;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1252;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1256;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1260;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1264;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1268;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1272;
    float PrePadding_TranslucentBasePass_Shared_Reflection_1276;
    float4 TranslucentBasePass_Shared_Reflection_SkyLightParameters;
    float TranslucentBasePass_Shared_Reflection_SkyLightCubemapBrightness;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1300;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1304;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1308;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1312;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1316;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1320;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1324;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1328;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1332;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1336;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1340;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1344;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1348;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1352;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1356;
    float4 TranslucentBasePass_Shared_PlanarReflection_ReflectionPlane;
    float4 TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionOrigin;
    float4 TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionXAxis;
    float4 TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionYAxis;
    float3x4 TranslucentBasePass_Shared_PlanarReflection_InverseTransposeMirrorMatrix;
    packed_float3 TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionParameters;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1484;
    float2 TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionParameters2;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1496;
    float PrePadding_TranslucentBasePass_Shared_PlanarReflection_1500;
    float4x4 TranslucentBasePass_Shared_PlanarReflection_ProjectionWithExtraFOV[2];
    float4 TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionScreenScaleBias[2];
    float2 TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionScreenBound;
    uint TranslucentBasePass_Shared_PlanarReflection_bIsStereo;
    float PrePadding_TranslucentBasePass_Shared_Fog_1676;
    float PrePadding_TranslucentBasePass_Shared_Fog_1680;
    float PrePadding_TranslucentBasePass_Shared_Fog_1684;
    float PrePadding_TranslucentBasePass_Shared_Fog_1688;
    float PrePadding_TranslucentBasePass_Shared_Fog_1692;
    float4 TranslucentBasePass_Shared_Fog_ExponentialFogParameters;
    float4 TranslucentBasePass_Shared_Fog_ExponentialFogParameters2;
    float4 TranslucentBasePass_Shared_Fog_ExponentialFogColorParameter;
    float4 TranslucentBasePass_Shared_Fog_ExponentialFogParameters3;
    float4 TranslucentBasePass_Shared_Fog_InscatteringLightDirection;
    float4 TranslucentBasePass_Shared_Fog_DirectionalInscatteringColor;
    float2 TranslucentBasePass_Shared_Fog_SinCosInscatteringColorCubemapRotation;
    float PrePadding_TranslucentBasePass_Shared_Fog_1800;
    float PrePadding_TranslucentBasePass_Shared_Fog_1804;
    packed_float3 TranslucentBasePass_Shared_Fog_FogInscatteringTextureParameters;
    float TranslucentBasePass_Shared_Fog_ApplyVolumetricFog;
    float PrePadding_TranslucentBasePass_1824;
    float PrePadding_TranslucentBasePass_1828;
    float PrePadding_TranslucentBasePass_1832;
    float PrePadding_TranslucentBasePass_1836;
    float PrePadding_TranslucentBasePass_1840;
    float PrePadding_TranslucentBasePass_1844;
    float PrePadding_TranslucentBasePass_1848;
    float PrePadding_TranslucentBasePass_1852;
    float PrePadding_TranslucentBasePass_1856;
    float PrePadding_TranslucentBasePass_1860;
    float PrePadding_TranslucentBasePass_1864;
    float PrePadding_TranslucentBasePass_1868;
    float PrePadding_TranslucentBasePass_1872;
    float PrePadding_TranslucentBasePass_1876;
    float PrePadding_TranslucentBasePass_1880;
    float PrePadding_TranslucentBasePass_1884;
    float PrePadding_TranslucentBasePass_1888;
    float PrePadding_TranslucentBasePass_1892;
    float PrePadding_TranslucentBasePass_1896;
    float PrePadding_TranslucentBasePass_1900;
    float PrePadding_TranslucentBasePass_1904;
    float PrePadding_TranslucentBasePass_1908;
    float PrePadding_TranslucentBasePass_1912;
    float PrePadding_TranslucentBasePass_1916;
    float PrePadding_TranslucentBasePass_1920;
    float PrePadding_TranslucentBasePass_1924;
    float PrePadding_TranslucentBasePass_1928;
    float PrePadding_TranslucentBasePass_1932;
    float PrePadding_TranslucentBasePass_1936;
    float PrePadding_TranslucentBasePass_1940;
    float PrePadding_TranslucentBasePass_1944;
    float PrePadding_TranslucentBasePass_1948;
    float PrePadding_TranslucentBasePass_1952;
    float PrePadding_TranslucentBasePass_1956;
    float PrePadding_TranslucentBasePass_1960;
    float PrePadding_TranslucentBasePass_1964;
    float PrePadding_TranslucentBasePass_1968;
    float PrePadding_TranslucentBasePass_1972;
    float PrePadding_TranslucentBasePass_1976;
    float PrePadding_TranslucentBasePass_1980;
    float PrePadding_TranslucentBasePass_1984;
    float PrePadding_TranslucentBasePass_1988;
    float PrePadding_TranslucentBasePass_1992;
    float PrePadding_TranslucentBasePass_1996;
    float PrePadding_TranslucentBasePass_2000;
    float PrePadding_TranslucentBasePass_2004;
    float PrePadding_TranslucentBasePass_2008;
    float PrePadding_TranslucentBasePass_2012;
    float PrePadding_TranslucentBasePass_2016;
    float PrePadding_TranslucentBasePass_2020;
    float PrePadding_TranslucentBasePass_2024;
    float PrePadding_TranslucentBasePass_2028;
    float PrePadding_TranslucentBasePass_2032;
    float PrePadding_TranslucentBasePass_2036;
    float PrePadding_TranslucentBasePass_2040;
    float PrePadding_TranslucentBasePass_2044;
    float PrePadding_TranslucentBasePass_2048;
    float PrePadding_TranslucentBasePass_2052;
    float PrePadding_TranslucentBasePass_2056;
    float PrePadding_TranslucentBasePass_2060;
    float PrePadding_TranslucentBasePass_2064;
    float PrePadding_TranslucentBasePass_2068;
    float PrePadding_TranslucentBasePass_2072;
    float PrePadding_TranslucentBasePass_2076;
    float PrePadding_TranslucentBasePass_2080;
    float PrePadding_TranslucentBasePass_2084;
    float PrePadding_TranslucentBasePass_2088;
    float PrePadding_TranslucentBasePass_2092;
    float PrePadding_TranslucentBasePass_2096;
    float PrePadding_TranslucentBasePass_2100;
    float PrePadding_TranslucentBasePass_2104;
    float PrePadding_TranslucentBasePass_2108;
    float PrePadding_TranslucentBasePass_2112;
    float PrePadding_TranslucentBasePass_2116;
    float PrePadding_TranslucentBasePass_2120;
    float PrePadding_TranslucentBasePass_2124;
    float PrePadding_TranslucentBasePass_2128;
    float PrePadding_TranslucentBasePass_2132;
    float PrePadding_TranslucentBasePass_2136;
    float PrePadding_TranslucentBasePass_2140;
    float4 TranslucentBasePass_HZBUvFactorAndInvFactor;
    float4 TranslucentBasePass_PrevScreenPositionScaleBias;
    float TranslucentBasePass_PrevSceneColorPreExposureInv;
};

struct type_Material
{
    float4 Material_VectorExpressions[2];
    float4 Material_ScalarExpressions[1];
};

constant float _108 = {};

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
    uint gl_SampleMask [[sample_mask]];
};

struct main0_in
{
    float4 in_var_TEXCOORD10_centroid [[user(locn0)]];
    float4 in_var_TEXCOORD11_centroid [[user(locn1)]];
    uint in_var_PRIMITIVE_ID [[user(locn2)]];
    float4 in_var_TEXCOORD7 [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], const device type_StructuredBuffer_v4float& View_PrimitiveSceneData [[buffer(1)]], constant type_TranslucentBasePass& TranslucentBasePass [[buffer(2)]], constant type_Material& Material [[buffer(3)]], texture3d<float> TranslucentBasePass_Shared_Fog_IntegratedLightScattering [[texture(0)]], sampler View_SharedBilinearClampedSampler [[sampler(0)]], float4 gl_FragCoord [[position]], uint gl_SampleMaskIn [[sample_mask]])
{
    main0_out out = {};
    float4 _137 = View.View_SVPositionToTranslatedWorld * float4(gl_FragCoord.xyz, 1.0);
    float3 _142 = (_137.xyz / float3(_137.w)) - float3(View.View_PreViewTranslation);
    bool _165 = TranslucentBasePass.TranslucentBasePass_Shared_Fog_ApplyVolumetricFog > 0.0;
    float4 _215;
    if (_165)
    {
        float4 _172 = View.View_WorldToClip * float4(_142, 1.0);
        float _173 = _172.w;
        float4 _202;
        if (_165)
        {
            _202 = TranslucentBasePass_Shared_Fog_IntegratedLightScattering.sample(View_SharedBilinearClampedSampler, float3(((_172.xy / float2(_173)).xy * float2(0.5, -0.5)) + float2(0.5), (log2((_173 * View.View_VolumetricFogGridZParams[0]) + View.View_VolumetricFogGridZParams[1]) * View.View_VolumetricFogGridZParams[2]) * View.View_VolumetricFogInvGridSize[2]), level(0.0));
        }
        else
        {
            _202 = float4(0.0, 0.0, 0.0, 1.0);
        }
        _215 = float4(_202.xyz + (in.in_var_TEXCOORD7.xyz * float3(_202.w)), _202.w * in.in_var_TEXCOORD7.w);
    }
    else
    {
        _215 = in.in_var_TEXCOORD7;
    }
    float3 _216 = fast::max(Material.Material_VectorExpressions[1].xyz * float3(((1.0 + dot(float3(-1.0, -1.5, 3.0) / float3(sqrt(12.25)), fast::normalize(float3x3(in.in_var_TEXCOORD10_centroid.xyz, cross(in.in_var_TEXCOORD11_centroid.xyz, in.in_var_TEXCOORD10_centroid.xyz) * float3(in.in_var_TEXCOORD11_centroid.w), in.in_var_TEXCOORD11_centroid.xyz) * fast::normalize((float3(0.0, 0.0, 1.0) * float3(View.View_NormalOverrideParameter.w)) + View.View_NormalOverrideParameter.xyz)))) * 0.5) + 0.20000000298023223876953125), float3(0.0));
    float3 _246;
    if (View.View_OutOfBoundsMask > 0.0)
    {
        uint _222 = in.in_var_PRIMITIVE_ID * 26u;
        float3 _245;
        if (any(abs(_142 - View_PrimitiveSceneData._m0[_222 + 5u].xyz) > (View_PrimitiveSceneData._m0[_222 + 19u].xyz + float3(1.0))))
        {
            _245 = mix(float3(1.0, 1.0, 0.0), float3(0.0, 1.0, 1.0), float3(float3(fract(dot(_142, float3(0.57700002193450927734375)) * 0.00200000009499490261077880859375)) > float3(0.5)));
        }
        else
        {
            _245 = _216;
        }
        _246 = _245;
    }
    else
    {
        _246 = _216;
    }
    float4 _255 = float4((_246 * float3(_215.w)) + _215.xyz, _108);
    _255.w = 1.0;
    float4 _268;
    uint _269;
    if (View.View_NumSceneColorMSAASamples > 1)
    {
        _268 = _255 * float4(float(View.View_NumSceneColorMSAASamples) * 0.25);
        _269 = gl_SampleMaskIn & 15u;
    }
    else
    {
        _268 = _255;
        _269 = gl_SampleMaskIn;
    }
    out.out_var_SV_Target0 = _268;
    out.gl_SampleMask = _269;
    return out;
}

