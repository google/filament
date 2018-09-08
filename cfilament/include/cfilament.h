/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CFILAMENT_H__
#define __CFILAMENT_H__

#include <stdint.h>

// Use stdcall on Windows, cdecl/default on Linux
#ifdef WIN32
#define APICALL __stdcall
#else
#define APICALL
#endif

// When BUILD_CFILAMENT (which is defined by CMAKE automatically) is defined, we mark
// all our functions to be exported. Otherwise we try to mark them as imported.
#ifdef BUILD_CFILAMENT

#ifdef _MSC_VER
#define APIEXPORT __declspec(dllexport)
#else
#define APIEXPORT __attribute__ ((visibility ("default")))
#endif

#else

#ifdef _MSC_VER
#define APIEXPORT __declspec(dllimport)
#else
#define APIEXPORT
#endif

#endif // BUILD_CFILAMENT

#ifdef __cplusplus
extern "C" {
#endif

// Extra markers for code-introspection to denote out and inout arguments
#ifdef FILAMENT_PARSING_IDL
// Mark pointers as buffers that we only every write to
#define FARGOUT __attribute__((annotate("out")))
// Mark pointers as buffers that are both read/written
#define FARGINOUT __attribute__((annotate("inout")))
// Used to mark void* as opaque handles instead of buffers that we read/write
#define FARGOPAQUE __attribute__((annotate("opaque")))
#else
#define FARGOUT
#define FARGINOUT
#define FARGOPAQUE
#endif // FILAMENT_PARSING_IDL

// We use forward declarations for consumers to hide the actual types, but when
// the actual wrapper functions are compiled, these declarations are aliased to
// the real filament types they represent. This is controlled by this define.
#ifndef CFILAMENT_SKIP_DECL
typedef struct FCamera FCamera;
typedef struct FMat4 FMat4;
typedef struct FMat4f FMat4f;
typedef struct FFloat3 FFloat3;
typedef struct FFloat4 FFloat4;
typedef struct FBox FBox;
typedef struct FFrustum FFrustum;
typedef FFloat3 FLinearColor;
typedef FFloat4 FLinearColorA;

typedef struct FEngine FEngine;
typedef struct FSwapChain FSwapChain;
typedef struct FView FView;
typedef struct FRenderer FRenderer;
typedef struct FFence FFence;
typedef struct FScene FScene;
typedef struct FStream FStream;
typedef struct FIndexBuffer FIndexBuffer;
typedef struct FVertexBuffer FVertexBuffer;
typedef struct FIndirectLight FIndirectLight;
typedef struct FMaterial FMaterial;
typedef struct FMaterialInstance FMaterialInstance;
typedef struct FSkybox FSkybox;
typedef struct FTexture FTexture;
typedef struct FTextureBuilder FTextureBuilder;
typedef struct FTransformManager FTransformManager;
typedef struct FLightManager FLightManager;
typedef struct FRenderableManager FRenderableManager;
typedef struct FIndexBufferBuilder FIndexBufferBuilder;
typedef struct FIndirectLightBuilder FIndirectLightBuilder;
typedef struct FLightManagerBuilder FLightManagerBuilder;
typedef struct FVertexBufferBuilder FVertexBufferBuilder;
typedef struct FRenderableManagerBuilder FRenderableManagerBuilder;
typedef struct FRenderableManagerBone FRenderableManagerBone;
typedef struct FSkyboxBuilder FSkyboxBuilder;
typedef struct FStreamBuilder FStreamBuilder;

// These are serialized to integers
typedef int FEntity;

// These must match the base of the respective enum class
typedef int FCameraProjection;
typedef int FCameraFov;
typedef uint8_t FBackend;
typedef uint8_t FFenceType;
typedef uint8_t FFenceMode;
typedef int8_t FFenceStatus;
typedef uint8_t FIndexBufferIndexType;
typedef uint8_t FLightManagerType;
typedef uint8_t FMaterialShading;
typedef uint8_t FMaterialInterpolation;
typedef uint8_t FMaterialBlendingMode;
typedef uint8_t FMaterialVertexDomain;
typedef uint8_t FMaterialCullingMode;
typedef uint8_t FMaterialParameterType;
typedef uint8_t FMaterialSamplerType;
typedef uint8_t FMaterialPrecision;
typedef uint8_t FVertexAttribute;
typedef uint8_t FElementType;
typedef uint8_t FAntiAliasing;
typedef int8_t FDepthPrepass;
typedef uint8_t FPrimitiveType;
typedef uint8_t FPixelDataFormat;
typedef uint8_t FPixelDataType;
typedef uint16_t FTextureFormat;
typedef uint8_t FSamplerType;
typedef uint8_t FTextureUsage;
typedef uint8_t FTextureCubemapFace;
typedef uint8_t FSamplerWrapMode;
typedef uint8_t FSamplerMinFilter;
typedef uint8_t FSamplerMagFilter;
typedef uint8_t FSamplerCompareMode;
typedef uint8_t FSamplerCompareFunc;
typedef uint32_t FTextureSampler;
typedef uint64_t size_t;
typedef uint8_t FBool;
#endif // CFILAMENT_SKIP_DECL

// Equivalent to Material::ParameterInfo
typedef struct FParameter {
    const char *name;
    uint8_t is_sampler;
    FMaterialParameterType type;
    FMaterialSamplerType sampler_type;
    int count;
    FMaterialPrecision precision;
} FParameter;

// Equivalent to driver::FaceOffsets
typedef struct FFaceOffsets {
    size_t px;
    size_t nx;
    size_t py;
    size_t ny;
    size_t pz;
    size_t nz;
} FFaceOffsets;

// Must be in sync with BufferDescriptor::Callback
typedef void(__cdecl *FFreeBufferFn)(void *buffer, size_t buffer_size, FARGOPAQUE void *user);

APIEXPORT void APICALL Filament_Colors_Cct(float temperature, FARGOUT FLinearColor *color);
APIEXPORT void APICALL Filament_Colors_IlluminantD(float temperature, FARGOUT FLinearColor *color);

/**************************************************************************************************
 * filament::Camera
 *************************************************************************************************/
APIEXPORT void APICALL Filament_Camera_SetProjectionFrustum(FCamera *camera,
        FCameraProjection projection,
        double left, double right,
        double bottom, double top,
        double near, double far);
APIEXPORT void APICALL Filament_Camera_SetProjectionFov(FCamera *camera,
        double fovInDegrees, double aspect,
        double near, double far,
        FCameraFov fov);
APIEXPORT void APICALL Filament_Camera_SetProjectionLens(FCamera *camera,
        double focalLength, double near, double far);
APIEXPORT void APICALL Filament_Camera_SetProjectionMatrix(FCamera *camera,
        FMat4 *matrix,
        double near, double far);
APIEXPORT void APICALL Filament_Camera_GetProjectionMatrix(FCamera *camera, FARGOUT FMat4 *matrixOut);
APIEXPORT void APICALL Filament_Camera_GetCullingProjectionMatrix(FCamera *camera, FARGOUT FMat4 *matrixOut);
APIEXPORT float APICALL Filament_Camera_GetNear(FCamera *camera);
APIEXPORT float APICALL Filament_Camera_GetCullingFar(FCamera *camera);
APIEXPORT void APICALL Filament_Camera_SetModelMatrix(FCamera *camera, FMat4f *matrix);
APIEXPORT void APICALL Filament_Camera_LookAt(FCamera *camera, FFloat3 *eye, FFloat3 *center, FFloat3 *up);
APIEXPORT void APICALL Filament_Camera_GetModelMatrix(FCamera *camera, FARGOUT FMat4f *matrixOut);
APIEXPORT void APICALL Filament_Camera_GetViewMatrix(FCamera *camera, FARGOUT FMat4f *matrixOut);
APIEXPORT void APICALL Filament_Camera_GetPosition(FCamera *camera, FARGOUT FFloat3 *vectorOut);
APIEXPORT void APICALL Filament_Camera_GetLeftVector(FCamera *camera, FARGOUT FFloat3 *vectorOut);
APIEXPORT void APICALL Filament_Camera_GetUpVector(FCamera *camera, FARGOUT FFloat3 *vectorOut);
APIEXPORT void APICALL Filament_Camera_GetForwardVector(FCamera *camera, FARGOUT FFloat3 *vectorOut);
APIEXPORT void APICALL Filament_Camera_GetFrustum(FCamera *camera, FARGOUT FFrustum *frustumOut);
APIEXPORT FEntity APICALL Filament_Camera_GetEntity(FCamera *camera);
APIEXPORT void APICALL Filament_Camera_SetExposure(FCamera *camera, float aperture,
        float shutterSpeed, float sensitivity);
APIEXPORT float APICALL Filament_Camera_GetAperture(FCamera *camera);
APIEXPORT float APICALL Filament_Camera_GetShutterSpeed(FCamera *camera);
APIEXPORT float APICALL Filament_Camera_GetSensitivity(FCamera *camera);
APIEXPORT void APICALL Filament_Camera_InverseProjection(FMat4 *projection, FARGOUT FMat4 *invertedOut);
APIEXPORT void APICALL Filament_Camera_InverseProjectionF(FMat4f *projection, FARGOUT FMat4f *invertedOut);

/**************************************************************************************************
 * filament::Engine
 *************************************************************************************************/
APIEXPORT FEngine *APICALL Filament_Engine_Create(FBackend backend);
APIEXPORT FEngine *APICALL Filament_Engine_CreateShared(FBackend backend, FARGOPAQUE void *sharedGlContext);
APIEXPORT void APICALL Filament_Engine_DestroyEngine(FEngine *engine);

// SwapChain
APIEXPORT FSwapChain *APICALL Filament_Engine_CreateSwapChain(FEngine *engine, FARGOPAQUE void *surface, uint64_t flags);
APIEXPORT void APICALL Filament_Engine_DestroySwapChain(FEngine *engine, FSwapChain *swapChain);

// FView

APIEXPORT FView *APICALL Filament_Engine_CreateView(FEngine *engine);
APIEXPORT void APICALL Filament_Engine_DestroyView(FEngine *engine, FView *view);

// Renderer
APIEXPORT FRenderer *APICALL Filament_Engine_CreateRenderer(FEngine *engine);
APIEXPORT void APICALL Filament_Engine_DestroyRenderer(FEngine *engine, FRenderer *renderer);

// Camera
APIEXPORT FCamera *APICALL Filament_Engine_CreateCamera(FEngine *engine);
APIEXPORT FCamera *APICALL Filament_Engine_CreateCameraWithEntity(FEngine *engine, FEntity entity);

APIEXPORT void APICALL Filament_Engine_DestroyCamera(FEngine *engine, FCamera *camera);

// Scene
APIEXPORT FScene *APICALL Filament_Engine_CreateScene(FEngine *engine);
APIEXPORT void APICALL Filament_Engine_DestroyScene(FEngine *engine, FScene *scene);

// FFence
APIEXPORT FFence *APICALL Filament_Engine_CreateFence(FEngine *engine, FFenceType fenceType);
APIEXPORT void APICALL Filament_Engine_DestroyFence(FEngine *engine, FFence *fence);

// Stream

APIEXPORT void APICALL Filament_Engine_DestroyStream(FEngine *engine, FStream *stream);

// Others...

APIEXPORT void APICALL Filament_Engine_DestroyIndexBuffer(FEngine *engine, FIndexBuffer *indexBuffer);
APIEXPORT void APICALL Filament_Engine_DestroyVertexBuffer(FEngine *engine, FVertexBuffer *vertexBuffer);
APIEXPORT void APICALL Filament_Engine_DestroyIndirectLight(FEngine *engine, FIndirectLight *indirectLight);
APIEXPORT void APICALL Filament_Engine_DestroyMaterial(FEngine *engine, FMaterial *material);
APIEXPORT void APICALL Filament_Engine_DestroyMaterialInstance(FEngine *engine, FMaterialInstance *materialInstance);
APIEXPORT void APICALL Filament_Engine_DestroySkybox(FEngine *engine, FSkybox *skybox);
APIEXPORT void APICALL Filament_Engine_DestroyTexture(FEngine *engine, FTexture *texture);

// Managers...
APIEXPORT FTransformManager *APICALL Filament_Engine_GetTransformManager(FEngine *engine);
APIEXPORT FLightManager *APICALL Filament_Engine_GetLightManager(FEngine *engine);
APIEXPORT FRenderableManager *APICALL Filament_Engine_GetRenderableManager(FEngine *engine);

/**************************************************************************************************
 * filament::EntityManager
 *************************************************************************************************/
APIEXPORT void APICALL Filament_EntityManager_CreateEntities(FEntity *entities, int count);
APIEXPORT int APICALL Filament_EntityManager_CreateEntity();
APIEXPORT void APICALL Filament_EntityManager_DestroyEntities(FEntity *entities, int count);
APIEXPORT void APICALL Filament_EntityManager_DestroyEntity(FEntity entity);
APIEXPORT FBool APICALL Filament_EntityManager_IsAlive(FEntity entity);

/**************************************************************************************************
 * filament::Fence
 *************************************************************************************************/
APIEXPORT FFenceStatus APICALL Filament_Fence_Wait(FFence *fence, FFenceMode mode, uint64_t timeoutNanoSeconds);
APIEXPORT FFenceStatus APICALL Filament_Fence_WaitAndDestroy(FFence *fence, FFenceMode mode);

/**************************************************************************************************
 * filament::IndexBuffer
 *************************************************************************************************/
APIEXPORT FIndexBufferBuilder *APICALL Filament_IndexBuffer_CreateBuilder();
APIEXPORT void APICALL
Filament_IndexBuffer_DestroyBuilder(FIndexBufferBuilder *builder);
APIEXPORT void APICALL Filament_IndexBuffer_BuilderIndexCount(
        FIndexBufferBuilder *builder, uint32_t indexCount);
APIEXPORT void APICALL Filament_IndexBuffer_BuilderBufferType(
        FIndexBufferBuilder *builder, FIndexBufferIndexType indexType);
APIEXPORT FIndexBuffer *APICALL
Filament_IndexBuffer_BuilderBuild(FIndexBufferBuilder *builder, FEngine *engine);
APIEXPORT int APICALL Filament_IndexBuffer_GetIndexCount(FIndexBuffer *indexBuffer);
APIEXPORT int APICALL Filament_IndexBuffer_SetBuffer(
        FIndexBuffer *vertexBuffer, FEngine *engine, void *data, uint32_t sizeInBytes,
        uint32_t destOffsetInBytes, FFreeBufferFn freeBuffer, FARGOPAQUE void *freeBufferArg);

/**************************************************************************************************
 * filament::IndirectLight
 *************************************************************************************************/
APIEXPORT FIndirectLightBuilder *APICALL Filament_IndirectLight_CreateBuilder();

APIEXPORT void APICALL Filament_IndirectLight_DestroyBuilder(FIndirectLightBuilder *builder);

APIEXPORT FIndirectLight *APICALL
Filament_IndirectLight_BuilderBuild(FIndirectLightBuilder *builder, FEngine *engine);

APIEXPORT void APICALL Filament_IndirectLight_BuilderReflections(
        FIndirectLightBuilder *builder, FTexture *texture);

APIEXPORT void APICALL Filament_IndirectLight_Irradiance(FIndirectLightBuilder *builder,
        uint8_t bands,
        FFloat3 *sh);

APIEXPORT void APICALL Filament_IndirectLight_IrradianceAsTexture(
        FIndirectLightBuilder *builder, FTexture *texture);

APIEXPORT void APICALL Filament_IndirectLight_Intensity(FIndirectLightBuilder *builder,
        float envIntensity);

APIEXPORT void APICALL Filament_IndirectLight_Rotation(FIndirectLightBuilder *builder,
        float v0, float v1, float v2,
        float v3, float v4, float v5,
        float v6, float v7, float v8);

APIEXPORT void APICALL Filament_IndirectLight_SetIntensity(FIndirectLight *indirectLight,
        float intensity);

APIEXPORT float APICALL Filament_IndirectLight_GetIntensity(FIndirectLight *indirectLight);

APIEXPORT void APICALL Filament_IndirectLight_SetRotation(FIndirectLight *indirectLight,
        float v0, float v1, float v2,
        float v3, float v4, float v5,
        float v6, float v7, float v8);

/**************************************************************************************************
 * filament::LightManager
 *************************************************************************************************/
APIEXPORT FBool APICALL Filament_LightManager_HasComponent(FLightManager *lm, FEntity entity);

APIEXPORT int APICALL Filament_LightManager_GetInstance(FLightManager *lm, FEntity entity);

APIEXPORT void APICALL Filament_LightManager_Destroy(FLightManager *lm, FEntity entity);

APIEXPORT FLightManagerBuilder *APICALL Filament_LightManager_CreateBuilder(FLightManagerType lightType);

APIEXPORT void APICALL Filament_LightManager_DestroyBuilder(FLightManagerBuilder *builder);

APIEXPORT void APICALL Filament_LightManager_BuilderCastShadows(FLightManagerBuilder *builder, FBool enable);

APIEXPORT void APICALL Filament_LightManager_BuilderShadowOptions(
        FLightManagerBuilder *builder, uint32_t mapSize, float constantBias,
        float normalBias, float shadowFar);

APIEXPORT void APICALL
Filament_LightManager_BuilderCastLight(FLightManagerBuilder *builder, FBool enabled);

APIEXPORT void APICALL Filament_LightManager_BuilderPosition(FLightManagerBuilder *builder, FFloat3 *position);

APIEXPORT void APICALL Filament_LightManager_BuilderDirection(FLightManagerBuilder *builder, FFloat3 *direction);

APIEXPORT void APICALL Filament_LightManager_BuilderColor(FLightManagerBuilder *builder, FLinearColor color);

APIEXPORT void APICALL Filament_LightManager_BuilderIntensity(FLightManagerBuilder *builder, float intensity);

APIEXPORT void APICALL Filament_LightManager_BuilderIntensityWatts(
        FLightManagerBuilder *builder, float watts, float efficiency);

APIEXPORT void APICALL
Filament_LightManager_BuilderFalloff(FLightManagerBuilder *builder, float radius);

APIEXPORT void APICALL Filament_LightManager_BuilderSpotLightCone(
        FLightManagerBuilder *builder, float inner, float outer);

APIEXPORT void APICALL Filament_LightManager_BuilderAngularRadius(
        FLightManagerBuilder *builder, float angularRadius);

APIEXPORT void APICALL
Filament_LightManager_BuilderHaloSize(FLightManagerBuilder *builder, float haloSize);

APIEXPORT void APICALL Filament_LightManager_BuilderHaloFalloff(
        FLightManagerBuilder *builder, float haloFalloff);

APIEXPORT FBool APICALL Filament_LightManager_BuilderBuild(FLightManagerBuilder *builder,
        FEngine *engine,
        FEntity entity);

APIEXPORT void APICALL Filament_LightManager_SetPosition(FLightManager *lm, int i,
        FFloat3 *position);

APIEXPORT void APICALL Filament_LightManager_GetPosition(FLightManager *lm, int i,
        FFloat3 *out);

APIEXPORT void APICALL Filament_LightManager_SetDirection(FLightManager *lm, int i,
        FFloat3 *direction);

APIEXPORT void APICALL Filament_LightManager_GetDirection(FLightManager *lm, int i,
        FFloat3 *out);

APIEXPORT void APICALL Filament_LightManager_SetColor(FLightManager *lm, int i,
        float linearR, float linearG,
        float linearB);

APIEXPORT void APICALL Filament_LightManager_GetColor(FLightManager *lm, int i,
        FFloat3 *out);

APIEXPORT void APICALL Filament_LightManager_SetIntensity(FLightManager *lm, int i,
        float intensity);

APIEXPORT void APICALL Filament_LightManager_SetIntensityWatts(FLightManager *lm, int i,
        float watts,
        float efficiency);

APIEXPORT float APICALL Filament_LightManager_GetIntensity(FLightManager *lm, int i);

APIEXPORT void APICALL Filament_LightManager_SetFalloff(FLightManager *lm, int i,
        float falloff);

APIEXPORT float APICALL Filament_LightManager_GetFalloff(FLightManager *lm, int i);

APIEXPORT void APICALL Filament_LightManager_SetSpotLightCone(FLightManager *lm, int i,
        float inner, float outer);

APIEXPORT void APICALL Filament_LightManager_SetSunAngularRadius(FLightManager *lm, int i,
        float angularRadius);

APIEXPORT float APICALL Filament_LightManager_GetSunAngularRadius(FLightManager *lm,
        int i);

APIEXPORT void APICALL Filament_LightManager_SetSunHaloSize(FLightManager *lm, int i,
        float haloSize);

APIEXPORT float APICALL Filament_LightManager_GetHaloSize(FLightManager *lm, int i);

APIEXPORT void APICALL Filament_LightManager_SetSunHaloFalloff(FLightManager *lm, int i,
        float haloFalloff);

APIEXPORT float APICALL Filament_LightManager_GetHaloFalloff(FLightManager *lm, int i);

/**************************************************************************************************
 * filament::Material
 *************************************************************************************************/
APIEXPORT FMaterial *APICALL Filament_Material_BuilderBuild(FEngine *engine, void *buffer,
        int size);

APIEXPORT FMaterialInstance *APICALL
Filament_Material_GetDefaultInstance(FMaterial *material);

APIEXPORT FMaterialInstance *APICALL
Filament_Material_CreateInstance(FMaterial *material);

APIEXPORT const char *APICALL Filament_Material_GetName(FMaterial *material);

APIEXPORT FMaterialShading APICALL Filament_Material_GetShading(FMaterial *material);

APIEXPORT FMaterialInterpolation APICALL Filament_Material_GetInterpolation(FMaterial *material);

APIEXPORT FMaterialBlendingMode APICALL Filament_Material_GetBlendingMode(FMaterial *material);

APIEXPORT FMaterialVertexDomain APICALL Filament_Material_GetVertexDomain(FMaterial *material);

APIEXPORT FMaterialCullingMode APICALL Filament_Material_GetCullingMode(FMaterial *material);

APIEXPORT FBool APICALL Filament_Material_IsColorWriteEnabled(FMaterial *material);

APIEXPORT FBool APICALL Filament_Material_IsDepthWriteEnabled(FMaterial *material);

APIEXPORT FBool APICALL Filament_Material_IsDepthCullingEnabled(FMaterial *material);

APIEXPORT FBool APICALL Filament_Material_IsDoubleSided(FMaterial *material);

APIEXPORT float APICALL Filament_Material_GetMaskThreshold(FMaterial *material);

APIEXPORT int APICALL Filament_Material_GetParameterCount(FMaterial *material);

APIEXPORT void APICALL Filament_Material_GetParameters(FMaterial *material, FParameter *paramsOut, int count);

APIEXPORT uint32_t APICALL Filament_Material_GetRequiredAttributes(FMaterial *material);

APIEXPORT FBool APICALL Filament_Material_HasParameter(FMaterial *material, const char *name);

/**************************************************************************************************
 * filament::MaterialInstance
 *************************************************************************************************/
APIEXPORT void APICALL Filament_MaterialInstance_SetParameterBool(FMaterialInstance *instance, char *name, FBool x);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterBool2(FMaterialInstance *instance, char *name, FBool x, FBool y);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterBool3(FMaterialInstance *instance, char *name, FBool x, FBool y, FBool z);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterBool4(FMaterialInstance *instance, char *name, FBool x, FBool y, FBool z,
        FBool w);
APIEXPORT void APICALL Filament_MaterialInstance_SetParameterInt(FMaterialInstance *instance, char *name, int x);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterInt2(FMaterialInstance *instance, char *name, int x, int y);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterInt3(FMaterialInstance *instance, char *name, int x, int y, int z);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterInt4(FMaterialInstance *instance, char *name, int x, int y, int z, int w);
APIEXPORT void APICALL Filament_MaterialInstance_SetParameterFloat(FMaterialInstance *instance, char *name, float x);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterFloat2(FMaterialInstance *instance, char *name, float x, float y);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterFloat3(FMaterialInstance *instance, char *name, float x, float y, float z);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterFloat4(FMaterialInstance *instance, char *name, float x, float y, float z,
        float w);
APIEXPORT void APICALL
Filament_MaterialInstance_SetBooleanParameterArray(FMaterialInstance *instance, char *name, FBool *v, size_t count);
APIEXPORT void APICALL
Filament_MaterialInstance_SetIntParameterArray(FMaterialInstance *instance, char *name, int32_t *v, size_t count);
APIEXPORT void APICALL
Filament_MaterialInstance_SetFloatParameterArray(FMaterialInstance *instance, char *name, float *v, size_t count);
APIEXPORT void APICALL
Filament_MaterialInstance_SetParameterTexture(FMaterialInstance *instance, char *name, FTexture *texture, int sampler_);
APIEXPORT void APICALL Filament_MaterialInstance_SetScissor(FMaterialInstance *instance,
        uint32_t left, uint32_t bottom,
        uint32_t width, uint32_t height);
APIEXPORT void APICALL Filament_MaterialInstance_UnsetScissor(FMaterialInstance *instance);

/**************************************************************************************************
 * filament::RenderableManager
 *************************************************************************************************/
APIEXPORT FBool APICALL Filament_RenderableManager_HasComponent(FRenderableManager *rm, FEntity entity);
APIEXPORT uint32_t APICALL Filament_RenderableManager_GetInstance(FRenderableManager *rm, FEntity entity);
APIEXPORT void APICALL Filament_RenderableManager_Destroy(FRenderableManager *rm, FEntity entity);
APIEXPORT FRenderableManagerBuilder *APICALL Filament_RenderableManager_CreateBuilder(size_t count);
APIEXPORT void APICALL Filament_RenderableManager_DestroyBuilder(FRenderableManagerBuilder *builder);
APIEXPORT FBool APICALL Filament_RenderableManager_BuilderBuild(
        FRenderableManagerBuilder *builder, FEngine *engine, FEntity entity);
APIEXPORT void APICALL Filament_RenderableManager_BuilderGeometry1(
        FRenderableManagerBuilder *builder, size_t index,
        FPrimitiveType primitiveType, FVertexBuffer *vertexBuffer,
        FIndexBuffer *indexBuffer);
APIEXPORT void APICALL Filament_RenderableManager_BuilderGeometry2(
        FRenderableManagerBuilder *builder, size_t index,
        FPrimitiveType primitiveType, FVertexBuffer *vertexBuffer,
        FIndexBuffer *indexBuffer, size_t offset, size_t count);
APIEXPORT void APICALL Filament_RenderableManager_BuilderGeometry3(
        FRenderableManagerBuilder *builder, size_t index,
        FPrimitiveType primitiveType, FVertexBuffer *vertexBuffer,
        FIndexBuffer *indexBuffer, size_t offset, size_t minIndex, size_t maxIndex,
        size_t count);
APIEXPORT void APICALL Filament_RenderableManager_BuilderMaterial(
        FRenderableManagerBuilder *builder, size_t index,
        FMaterialInstance *materialInstance);
APIEXPORT void APICALL Filament_RenderableManager_BuilderBlendOrder(
        FRenderableManagerBuilder *builder, size_t index, uint16_t blendOrder);
APIEXPORT void APICALL Filament_RenderableManager_BuilderBoundingBox(
        FRenderableManagerBuilder *builder, FBox *box);
APIEXPORT void APICALL Filament_RenderableManager_BuilderLayerMask(
        FRenderableManagerBuilder *builder, uint8_t select, uint8_t value);
APIEXPORT void APICALL Filament_RenderableManager_BuilderPriority(
        FRenderableManagerBuilder *builder, uint8_t priority);
APIEXPORT void APICALL Filament_RenderableManager_BuilderCulling(
        FRenderableManagerBuilder *builder, FBool enabled);
APIEXPORT void APICALL Filament_RenderableManager_BuilderCastShadows(
        FRenderableManagerBuilder *builder, FBool enabled);
APIEXPORT void APICALL Filament_RenderableManager_BuilderReceiveShadows(
        FRenderableManagerBuilder *builder, FBool enabled);
APIEXPORT void APICALL Filament_RenderableManager_BuilderSkinning(
        FRenderableManagerBuilder *builder, size_t boneCount);
APIEXPORT int APICALL Filament_RenderableManager_BuilderSkinningBones(
        FRenderableManagerBuilder *builder, FRenderableManagerBone *bones,
        size_t boneCount);
APIEXPORT int APICALL Filament_RenderableManager_SetBonesAsMatrices(
        FRenderableManager *rm, uint32_t i, FMat4f *matrices, size_t boneCount,
        size_t offset);
APIEXPORT int APICALL Filament_RenderableManager_SetBonesAsQuaternions(
        FRenderableManager *rm, uint32_t i, FRenderableManagerBone *bones,
        size_t boneCount, size_t offset);
APIEXPORT void APICALL Filament_RenderableManager_SetAxisAlignedBoundingBox(
        FRenderableManager *rm, uint32_t i, float cx, float cy, float cz, float ex,
        float ey, float ez);
APIEXPORT void APICALL Filament_RenderableManager_SetLayerMask(FRenderableManager *rm,
        uint32_t i,
        uint8_t select,
        uint8_t value);
APIEXPORT void APICALL Filament_RenderableManager_SetPriority(FRenderableManager *rm,
        uint32_t i,
        uint8_t priority);
APIEXPORT void APICALL Filament_RenderableManager_SetCastShadows(FRenderableManager *rm,
        uint32_t i,
        FBool enabled);
APIEXPORT void APICALL Filament_RenderableManager_SetReceiveShadows(
        FRenderableManager *rm, uint32_t i, FBool enabled);
APIEXPORT FBool APICALL Filament_RenderableManager_IsShadowCaster(FRenderableManager *rm,
        uint32_t i);
APIEXPORT FBool APICALL Filament_RenderableManager_IsShadowReceiver(FRenderableManager *rm,
        uint32_t i);
APIEXPORT void APICALL Filament_RenderableManager_GetAxisAlignedBoundingBox(
        FRenderableManager *rm, uint32_t i, FBox *aabbOut);
APIEXPORT int APICALL Filament_RenderableManager_GetPrimitiveCount(FRenderableManager *rm,
        uint32_t i);
APIEXPORT void APICALL Filament_RenderableManager_SetMaterialInstanceAt(
        FRenderableManager *rm, uint32_t i, size_t primitiveIndex,
        FMaterialInstance *materialInstance);
APIEXPORT void APICALL Filament_RenderableManager_SetGeometryAt1(
        FRenderableManager *rm, uint32_t i, size_t primitiveIndex,
        FPrimitiveType primitiveType, FVertexBuffer *vertexBuffer,
        FIndexBuffer *indexBuffer, size_t offset, size_t count);
APIEXPORT void APICALL Filament_RenderableManager_SetGeometryAt2(
        FRenderableManager *rm, uint32_t i, size_t primitiveIndex,
        FPrimitiveType primitiveType, size_t offset,
        size_t count);
APIEXPORT void APICALL Filament_RenderableManager_SetBlendOrderAt(FRenderableManager *rm,
        uint32_t i,
        size_t primitiveIndex,
        uint16_t blendOrder);
APIEXPORT uint32_t APICALL Filament_RenderableManager_GetEnabledAttributesAt(
        FRenderableManager *rm, uint32_t i, size_t primitiveIndex);

/**************************************************************************************************
 * filament::Renderer
 *************************************************************************************************/
APIEXPORT FBool APICALL Filament_Renderer_BeginFrame(FRenderer *renderer,
        FSwapChain *swapChain);

APIEXPORT void APICALL Filament_Renderer_EndFrame(FRenderer *renderer);

APIEXPORT void APICALL Filament_Renderer_Render(FRenderer *renderer, FView *view);

/**************************************************************************************************
 * filament::Scene
 *************************************************************************************************/
APIEXPORT void APICALL Filament_Scene_SetSkybox(FScene *scene, FSkybox *skybox);

APIEXPORT void APICALL Filament_Scene_SetIndirectLight(FScene *scene,
        FIndirectLight *indirectLight);

APIEXPORT void APICALL Filament_Scene_AddEntity(FScene *scene, FEntity entity);

APIEXPORT void APICALL Filament_Scene_Remove(FScene *scene, FEntity entity);

APIEXPORT int APICALL Filament_Scene_GetRenderableCount(FScene *scene);

APIEXPORT int APICALL Filament_Scene_GetLightCount(FScene *scene);

/**************************************************************************************************
 * filament::TransformManager
 *************************************************************************************************/
APIEXPORT FBool APICALL Filament_TransformManager_HasComponent(FTransformManager *tm,
        FEntity entity);

APIEXPORT int APICALL Filament_TransformManager_GetInstance(FTransformManager *tm,
        FEntity entity);

APIEXPORT int APICALL Filament_TransformManager_CreateUninitialized(FTransformManager *tm,
        FEntity entity);

APIEXPORT int APICALL Filament_TransformManager_Create(FTransformManager *tm,
        FEntity entity, int parent,
        FMat4f *localTransform);

APIEXPORT void APICALL Filament_TransformManager_Destroy(FTransformManager *tm,
        FEntity entity);

APIEXPORT void APICALL Filament_TransformManager_SetParent(FTransformManager *tm, int i,
        int newParent);

APIEXPORT void APICALL Filament_TransformManager_SetTransform(
        FTransformManager *tm, int i, FMat4f *localTransform);

APIEXPORT void APICALL Filament_TransformManager_GetTransform(
        FTransformManager *tm, int i, FMat4f *outLocalTransform);

APIEXPORT void APICALL Filament_TransformManager_GetWorldTransform(
        FTransformManager *tm, int i, FMat4f *outWorldTransform);

APIEXPORT void APICALL
Filament_TransformManager_OpenLocalTransformTransaction(FTransformManager *tm);

APIEXPORT void APICALL
Filament_TransformManager_CommitLocalTransformTransaction(FTransformManager *tm);

/**************************************************************************************************
 * filament::VertexBuffer
 *************************************************************************************************/
APIEXPORT FVertexBufferBuilder *APICALL Filament_VertexBuffer_CreateBuilder();

APIEXPORT void APICALL
Filament_VertexBuffer_DestroyBuilder(FVertexBufferBuilder *builder);

APIEXPORT void APICALL Filament_VertexBuffer_BuilderVertexCount(
        FVertexBufferBuilder *builder, uint32_t vertexCount);

APIEXPORT void APICALL Filament_VertexBuffer_BuilderBufferCount(
        FVertexBufferBuilder *builder, uint8_t bufferCount);

APIEXPORT void APICALL Filament_VertexBuffer_BuilderAttribute(
        FVertexBufferBuilder *builder, FVertexAttribute attribute,
        uint8_t bufferIndex, FElementType attributeType,
        uint32_t byteOffset, uint8_t byteStride);

APIEXPORT void APICALL Filament_VertexBuffer_BuilderNormalized(
        FVertexBufferBuilder *builder, FVertexAttribute attribute);

APIEXPORT FVertexBuffer *APICALL
Filament_VertexBuffer_BuilderBuild(FVertexBufferBuilder *builder, FEngine *engine);

APIEXPORT int APICALL Filament_VertexBuffer_GetVertexCount(FVertexBuffer *vertexBuffer);

APIEXPORT int APICALL Filament_VertexBuffer_SetBufferAt(
        FVertexBuffer *vertexBuffer, FEngine *engine, uint8_t bufferIndex, void *data,
        uint32_t sizeInBytes, uint32_t destOffsetInBytes, FFreeBufferFn freeBuffer,
        FARGOPAQUE void *freeBufferArg);

/**************************************************************************************************
 * filament::View
 *************************************************************************************************/
APIEXPORT void APICALL Filament_View_SetName(FView *view, const char *name);

APIEXPORT const char *APICALL Filament_View_GetName(FView *view);

APIEXPORT void APICALL Filament_View_SetScene(FView *view, FScene *scene);

APIEXPORT void APICALL Filament_View_SetCamera(FView *view, FCamera *camera);

APIEXPORT void APICALL Filament_View_SetViewport(FView *view, int left, int bottom,
        uint32_t width, uint32_t height);

APIEXPORT void APICALL Filament_View_SetClearColor(FView *view, FLinearColorA color);

APIEXPORT void APICALL Filament_View_GetClearColor(FView *view, FLinearColorA *colorOut);

APIEXPORT void APICALL Filament_View_SetClearTargets(FView *view, FBool color, FBool depth,
        FBool stencil);

APIEXPORT void APICALL Filament_View_SetVisibleLayers(FView *view, uint8_t select,
        uint8_t value);

APIEXPORT void APICALL Filament_View_SetShadowsEnabled(FView *view, FBool enabled);

APIEXPORT void APICALL Filament_View_SetSampleCount(FView *view, uint8_t count);

APIEXPORT uint8_t APICALL Filament_View_GetSampleCount(FView *view);

APIEXPORT void APICALL Filament_View_SetAntiAliasing(FView *view,
        FAntiAliasing type);

APIEXPORT FAntiAliasing APICALL Filament_View_GetAntiAliasing(FView *view);

typedef struct FDynamicResolutionOptions {
    FBool enabled;
    FBool homogeneousScaling;
    float targetFrameTimeMilli;
    float headRoomRatio;
    float scaleRate;
    float minScale;
    float maxScale;
    uint8_t history;
} FDynamicResolutionOptions;

APIEXPORT void APICALL Filament_View_SetDynamicResolutionOptions(
        FView *view, FDynamicResolutionOptions *optionsIn);

APIEXPORT void APICALL Filament_View_GetDynamicResolutionOptions(
        FView *view, FDynamicResolutionOptions *optionsOut);

APIEXPORT void APICALL Filament_View_SetDynamicLightingOptions(FView *view,
        float zLightNear,
        float zLightFar);

APIEXPORT void APICALL Filament_View_SetDepthPrepass(FView *view,
        FDepthPrepass value);

/**************************************************************************************************
 * filament::Skybox
 *************************************************************************************************/
APIEXPORT FSkyboxBuilder *APICALL Filament_Skybox_CreateBuilder();
APIEXPORT void APICALL Filament_Skybox_DestroyBuilder(FSkyboxBuilder *builder);
APIEXPORT void APICALL Filament_Skybox_BuilderEnvironment(FSkyboxBuilder *builder, FTexture *texture);
APIEXPORT void APICALL Filament_Skybox_BuilderShowSun(FSkyboxBuilder *builder, FBool show);
APIEXPORT FSkybox *APICALL Filament_Skybox_BuilderBuild(FSkyboxBuilder *builder, FEngine *engine);
APIEXPORT void APICALL Filament_Skybox_SetLayerMask(FSkybox *skybox, uint8_t select, uint8_t value);
APIEXPORT uint8_t APICALL Filament_Skybox_GetLayerMask(FSkybox *skybox);

/**************************************************************************************************
 * filament::Stream
 *************************************************************************************************/
APIEXPORT FStreamBuilder *APICALL Filament_Stream_Builder_Create();
APIEXPORT void APICALL Filament_Stream_Builder_Destroy(FStreamBuilder *builder);
APIEXPORT void APICALL Filament_Stream_Builder_StreamNative(FStreamBuilder *builder, void *stream);
APIEXPORT void APICALL Filament_Stream_Builder_StreamCopy(FStreamBuilder *builder, intptr_t externalTextureId);
APIEXPORT void APICALL Filament_Stream_Builder_Width(FStreamBuilder *builder, uint32_t width);
APIEXPORT void APICALL Filament_Stream_Builder_Height(FStreamBuilder *builder, uint32_t height);
APIEXPORT FStream *APICALL Filament_Stream_Builder_Build(FStreamBuilder *builder, FEngine *engine);

typedef struct {
    void *buffer;
    size_t size;
    FPixelDataFormat format;
    FPixelDataType type;
    uint8_t alignment;
    uint32_t left;
    uint32_t top;
    uint32_t stride;
    FFreeBufferFn freeBufferCallback;
    FARGOPAQUE void *freeBufferArg;
} FPixelBufferDescriptor;

APIEXPORT FBool APICALL Filament_Stream_IsNativeStream(FStream *stream);
APIEXPORT void APICALL Filament_Stream_SetDimensions(FStream *stream, uint32_t width, uint32_t height);
APIEXPORT void APICALL Filament_Stream_ReadPixels(FStream *stream, uint32_t xoffset, uint32_t yoffset,
        uint32_t width, uint32_t height,
        FPixelBufferDescriptor *buffer);

/**************************************************************************************************
 * filament::Texture
 *************************************************************************************************/
APIEXPORT FBool APICALL Filament_Texture_IsFormatSupported(FEngine *engine, FTextureFormat format);
APIEXPORT size_t APICALL Filament_Texture_ComputeDataSize(FPixelDataFormat format, FPixelDataType type,
        size_t stride, size_t height, size_t alignment);

APIEXPORT FTextureBuilder *APICALL Filament_Texture_Builder_Create();
APIEXPORT void APICALL Filament_Texture_Builder_Destroy(FTextureBuilder *builder);
APIEXPORT void APICALL Filament_Texture_Builder_Width(FTextureBuilder *builder, uint32_t width);
APIEXPORT void APICALL Filament_Texture_Builder_Height(FTextureBuilder *builder, uint32_t height);
APIEXPORT void APICALL Filament_Texture_Builder_Depth(FTextureBuilder *builder, uint32_t depth);
APIEXPORT void APICALL Filament_Texture_Builder_Levels(FTextureBuilder *builder, uint8_t levels);
APIEXPORT void APICALL Filament_Texture_Builder_Sampler(FTextureBuilder *builder, FSamplerType sampler);
APIEXPORT void APICALL Filament_Texture_Builder_Format(FTextureBuilder *builder, FTextureFormat format);
APIEXPORT void APICALL Filament_Texture_Builder_Usage(FTextureBuilder *builder, FTextureUsage usage);
APIEXPORT FTexture *APICALL Filament_Texture_Builder_Build(FTextureBuilder *builder, FEngine *engine);

APIEXPORT size_t APICALL Filament_Texture_GetWidth(FTexture *texture, size_t level);
APIEXPORT size_t APICALL Filament_Texture_GetHeight(FTexture *texture, size_t level);
APIEXPORT size_t APICALL Filament_Texture_GetDepth(FTexture *texture, size_t level);
APIEXPORT size_t APICALL Filament_Texture_GetLevels(FTexture *texture);
APIEXPORT FSamplerType APICALL Filament_Texture_GetTarget(FTexture *texture);
APIEXPORT FTextureFormat APICALL Filament_Texture_GetFormat(FTexture *texture);
APIEXPORT void APICALL Filament_Texture_SetImage(FTexture *texture, FEngine *engine,
        size_t level,
        FPixelBufferDescriptor *buffer);
APIEXPORT void APICALL Filament_Texture_SetSubImage(FTexture *texture, FEngine *engine,
        size_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        FPixelBufferDescriptor *buffer);
APIEXPORT void APICALL Filament_Texture_SetCubeImage(FTexture *texture, FEngine *engine,
        size_t level,
        FPixelBufferDescriptor *buffer, FFaceOffsets *faceOffsets);
APIEXPORT void APICALL Filament_Texture_SetExternalImage(FTexture *texture, FEngine *engine, void *image);
APIEXPORT void APICALL Filament_Texture_SetExternalStream(FTexture *texture, FEngine *engine, FStream *stream);
APIEXPORT void APICALL Filament_Texture_GenerateMipmaps(FTexture *texture, FEngine *engine);

typedef struct {
    FSamplerMagFilter filterMag;
    FSamplerMinFilter filterMin;
    FSamplerWrapMode wrapS;
    FSamplerWrapMode wrapT;
    FSamplerWrapMode wrapR;
    float anisotropy;
    FSamplerCompareMode compareMode;
    FSamplerCompareFunc compareFunc;
} FSamplerParams;

// Samplers are represented as their driver-side representation, which is a 32-bit packaged integer
APIEXPORT FTextureSampler APICALL Filament_TextureSampler_Create(FSamplerParams *params);
APIEXPORT void APICALL Filament_TextureSampler_GetParams(FTextureSampler sampler, FSamplerParams *paramsOut);

#ifdef __cplusplus
};
#endif

#undef FARGOUT
#undef FARGINOUT
#undef FARGOPAQUE

#endif // __CFILAMENT_H__
