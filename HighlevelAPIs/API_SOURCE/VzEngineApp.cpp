#include "VzEngineApp.h"
#include "VzRenderPath.h"
#include "backend/VzAssetLoader.h"
#include "backend/VzAssetExporter.h"
#include "backend/VzMeshAssimp.h"
#include "VzNameComponents.hpp"

#include "FIncludes.h"

#include <filesystem>
#include <iostream>

extern Engine* gEngine;
extern Material* gMaterialTransparent; // do not release
extern vzm::VzEngineApp gEngineApp;
extern gltfio::MaterialProvider* gMaterialProvider;

namespace vzm
{
    using CameraManipulator = filament::camutils::Manipulator<float>;
    using Entity = utils::Entity;

    void getDescendants(const utils::Entity ett, std::vector<utils::Entity>& decendants)
    {
        auto& tcm = gEngine->getTransformManager();
        auto ins = tcm.getInstance(ett);
        for (auto it = tcm.getChildrenBegin(ins); it != tcm.getChildrenEnd(ins); it++)
        {
            utils::Entity ett_child = tcm.getEntity(*it);
            decendants.push_back(ett_child);
            getDescendants(ett_child, decendants);
        }
    };
    void getDescendants(const VID vid, std::vector<VID>& decendants)
    {
        auto& tcm = gEngine->getTransformManager();
        auto ins = tcm.getInstance(Entity::import(vid));
        for (auto it = tcm.getChildrenBegin(ins); it != tcm.getChildrenEnd(ins); it++)
        {
            utils::Entity ett_child = tcm.getEntity(*it);
            decendants.push_back(ett_child.getId());
            getDescendants(ett_child.getId(), decendants);
        }
    }
    void cubeToScene(const VID vidCubeRenderable, const VID vidCube)
    {
        const Entity ettCubeRenderable = Entity::import(vidCubeRenderable);
        SceneVID vid_scene = gEngineApp.GetSceneVidBelongTo(vidCube);
        if (vid_scene != INVALID_VID)
        {
            Scene* scene = gEngineApp.GetScene(vid_scene);
            assert(scene);
            if (!scene->hasEntity(ettCubeRenderable))
            {
                // safely rearrangement
                auto& scenes = *gEngineApp.GetScenes();
                for (auto& it : scenes)
                {
                    it.second->remove(ettCubeRenderable);
                }
                scene->addEntity(ettCubeRenderable);
            }
        }
    };
}

namespace vzm
{
#pragma region // VzTypesetter
    void VzTypesetter::Measure()
    {
        if (isMeasured)
        {
            return;
        }
        FontVID font = textFormat.font;
        VzFontRes* font_res = gEngineApp.GetFontRes(font);
        if ((font == INVALID_VID) || (text.empty()))
        {
            return;
        }
        for (wchar_t glyph : text)
        {
            glyphCodes.emplace_back(glyph);
        }
        textWidth = MeasureLinesWidth(font);
        int32_t numberOfLines = linesWidth.size();
        textHeight = font_res->GetLineHeight() * numberOfLines;
        textHeight += (numberOfLines - 1) * textFormat.leading;
        isMeasured = true;
    }
    int32_t VzTypesetter::MeasureLinesWidth(FontVID font)
    {
        VzFontRes* font_res = gEngineApp.GetFontRes(font);
        int32_t lineMaxWidth = 0;
        int32_t lineWidth = 0;
        int32_t wordWidth = 0;
        int32_t kerning = textFormat.kerning;
        for (uint32_t glyphCode : glyphCodes)
        {
            if (font_res->IsNewLine(glyphCode))
            {
                lineWidth = lineWidth - kerning;
                lineMaxWidth = std::max(lineWidth, lineMaxWidth);
                linesWidth.emplace_back(lineWidth);
                lineWidth = 0;
                wordWidth = 0;
            }
            else
            {
                int32_t advanceX = font_res->GetAdvanceX(glyphCode);
                lineWidth += advanceX;
                if (font_res->IsSpace(glyphCode))
                {
                    wordWidth = 0;
                }
                else
                {
                    wordWidth += advanceX;
                }
                if ((fixedWidth > 0) && (fixedWidth < lineWidth))
                {
                    if (lineWidth > wordWidth)
                    {
                        lineWidth -= wordWidth;
                    }
                    else
                    {
                        lineWidth -= advanceX;
                        wordWidth = advanceX;
                    }
                    lineWidth = lineWidth - kerning;
                    lineMaxWidth = std::max(lineWidth, lineMaxWidth);
                    linesWidth.emplace_back(lineWidth);
                    lineWidth = wordWidth;
                }
            }
        }
        lineWidth = lineWidth - kerning;
        lineMaxWidth = std::max(lineWidth, lineMaxWidth);
        if (lineWidth > 0)
        {
            linesWidth.emplace_back(lineWidth);
        }
        return lineMaxWidth;
    }
    void VzTypesetter::Typeset()
    {
        if (texture)
        {
            return;
        }
        Measure();
        FontVID font = textFormat.font;
        VzFontRes* font_res = gEngineApp.GetFontRes(font);
        int32_t width = (fixedWidth > 0) ? fixedWidth : textWidth;
        int32_t height = (fixedHeight > 0) ? fixedHeight : textHeight;
        if ((font == INVALID_VID) || (width <= 0) || (height <= 0))
        {
            return;
        }
        uint8_t* pixels = new uint8_t[width * height];
        TextAlign textAlign = textFormat.textAlign;
        int32_t numberOfLines = linesWidth.size();
        int32_t lineX = GetLeftBlankWidth(textAlign, linesWidth[0], width);
        int32_t lineY = GetTopBlankHeight(textAlign, textHeight, height);
        int32_t lineWidthStack = 0;
        int32_t lineHeight = font_res->GetLineHeight();
        int32_t lineIndex = 0;
        int32_t baselineY = lineHeight * 3 / 4;
        for (uint32_t glyphCode : glyphCodes)
        {
            if (font_res->IsNewLine(glyphCode))
            {
                continue;
            }
            int32_t bearingX = font_res->GetBearingX(glyphCode);
            int32_t bearingY = baselineY - font_res->GetBearingY(glyphCode);
            int32_t glyphWidth = font_res->GetGlyphWidth(glyphCode);
            int32_t glyphHeight = font_res->GetGlyphHeight(glyphCode);
            const uint8_t* glyphPixels = font_res->GetGlyphPixels(glyphCode);
            for (int32_t glyphY = 0; glyphY < glyphHeight; glyphY++)
            {
                for (int32_t glyphX = 0; glyphX < glyphWidth; glyphX++)
                {
                    int32_t x = lineX + glyphX + bearingX;
                    int32_t y = lineY + glyphY + bearingY;
                    if ((x < 0) || (x >= width) || (y < 0) || (y >= height))
                    {
                        continue;
                    }
                    int32_t index = (y * width) + x;
                    uint16_t pixel = pixels[index];
                    pixel += glyphPixels[(glyphWidth * glyphY) + glyphX];
                    pixel = (pixel < 0xff) ? pixel : 0xff;
                    pixels[index] = (uint8_t) pixel;
                }
            }
            int32_t advanceX = font_res->GetAdvanceX(glyphCode) + textFormat.kerning;
            lineWidthStack += advanceX;
            if (lineWidthStack < linesWidth[lineIndex])
            {
                lineX += advanceX;
            }
            else
            {
                lineWidthStack = 0;
                lineIndex++;
                if (lineIndex < numberOfLines)
                {
                    lineX = GetLeftBlankWidth(textAlign, linesWidth[lineIndex], width);
                    lineY += lineHeight;
                }
            }
        }
        PixelBufferDescriptor buffer(
            pixels, width * height,
            PixelDataFormat::R,
            PixelDataType::UBYTE,
            [](void* data, size_t, void*) { delete[] reinterpret_cast<uint8_t*>(data); }
        );
        texture = Texture::Builder()
            .width(width)
            .height(height)
            .levels(0xff)
            .format(Texture::InternalFormat::R8)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .build(*gEngine);
        texture->setImage(*gEngine, 0, std::move(buffer));
        texture->generateMipmaps(*gEngine);
    }
    int32_t VzTypesetter::GetLeftBlankWidth(const TextAlign textAlign, const int32_t lineWidth, const int32_t width)
    {
        switch (textAlign) {
            case TextAlign::RIGHT:
            case TextAlign::TOP_RIGHT:
            case TextAlign::MIDDLE_RIGHT:
            case TextAlign::BOTTOM_RIGHT:
                return width - lineWidth;
            case TextAlign::CENTER:
            case TextAlign::TOP_CENTER:
            case TextAlign::MIDDLE_CENTER:
            case TextAlign::BOTTOM_CENTER:
                return (width - lineWidth) / 2;
            default:
                return 0;
        }
    }
    int32_t VzTypesetter::GetTopBlankHeight(const TextAlign textAlign, const int32_t textHeight, const int32_t height)
    {
        switch (textAlign)
        {
            case TextAlign::MIDDLE_LEFT:
            case TextAlign::MIDDLE_CENTER:
            case TextAlign::MIDDLE_RIGHT:
                return (height - textHeight) / 2;
            case TextAlign::BOTTOM_LEFT:
            case TextAlign::BOTTOM_CENTER:
            case TextAlign::BOTTOM_RIGHT:
                return height - textHeight;
            default:
                return 0;
        }
    }
#pragma endregion

#pragma region // VzSceneRes
    VzSceneRes::VzSceneRes() { NewIBL(); };
    VzSceneRes::~VzSceneRes() { Destory(); };
    void VzSceneRes::Destory()
    {
        if (ibl_) {
            delete ibl_;
            ibl_ = nullptr;
        }
        if (lightmapCube_) {
            delete lightmapCube_;
            lightmapCube_ = nullptr;
        }
    }
    IBL* VzSceneRes::GetIBL() { return ibl_; }
    IBL* VzSceneRes::NewIBL()
    {
        if (ibl_) {
            delete ibl_;
        }
        ibl_ = new IBL(*gEngine);
        return ibl_;
    }
    Cube* VzSceneRes::GetLightmapCube()
    {
        if (lightmapCube_)
        {
            return lightmapCube_;
        }
        lightmapCube_ = new Cube(*gEngine, gMaterialTransparent, { 0, 1, 0 }, false);
        return lightmapCube_;
    }
#pragma endregion

#pragma region // VzCameraRes
    VzCameraRes::~VzCameraRes()
    {
        if (cameraCube_) {
            delete cameraCube_;
            cameraCube_ = nullptr;
        }
        if (cameraManipulator_) {
            delete cameraManipulator_;
            cameraManipulator_ = nullptr;
        }
    }
    void VzCameraRes::SetCamera(Camera* camera) { camera_ = camera; }
    Camera* VzCameraRes::GetCamera() { return camera_; }
    Cube* VzCameraRes::GetCameraCube()
    {
        if (cameraCube_)
        {
            return cameraCube_;
        }
        cameraCube_ = new Cube(*gEngine, gMaterialTransparent, { 1, 0, 0 });
        return cameraCube_;
    }
    void VzCameraRes::NewCameraManipulator(const VzCamera::Controller& camController)
    {
        camController_ = camController;
        delete cameraManipulator_;
        cameraManipulator_ = CameraManipulator::Builder()
            .targetPosition(camController_.targetPosition[0], camController_.targetPosition[1], camController_.targetPosition[2])
            .upVector(camController_.upVector[0], camController_.upVector[1], camController_.upVector[2])
            .zoomSpeed(camController_.zoomSpeed)

            .orbitHomePosition(camController_.orbitHomePosition[0], camController_.orbitHomePosition[1], camController_.orbitHomePosition[2])
            .orbitSpeed(camController_.orbitSpeed[0], camController_.orbitSpeed[1])

            .fovDirection(camController_.isVerticalFov ? camutils::Fov::VERTICAL : camutils::Fov::HORIZONTAL)
            .fovDegrees(camController_.fovDegrees)
            .farPlane(camController_.farPlane)
            .mapExtent(camController_.mapExtent[0], camController_.mapExtent[1])
            .mapMinDistance(camController_.mapMinDistance)

            .flightStartPosition(camController_.flightStartPosition[0], camController_.flightStartPosition[1], camController_.flightStartPosition[2])
            .flightStartOrientation(camController_.flightStartPitch, camController_.flightStartYaw)
            .flightMaxMoveSpeed(camController_.flightMaxSpeed)
            .flightSpeedSteps(camController_.flightSpeedSteps)
            .flightPanSpeed(camController_.flightPanSpeed[0], camController_.flightPanSpeed[1])
            .flightMoveDamping(camController_.flightMoveDamping)

            .groundPlane(camController_.groundPlane[0], camController_.groundPlane[1], camController_.groundPlane[2], camController_.groundPlane[3])
            .panning(camController_.panning)
            .build((camutils::Mode)camController_.mode);
    }
    VzCamera::Controller* VzCameraRes::GetCameraController()
    {
        return &camController_;
    }
    CameraManipulator* VzCameraRes::GetCameraManipulator()
    {
        return cameraManipulator_;
    }
    void VzCameraRes::UpdateCameraWithCM(float deltaTime)
    {
        if (cameraManipulator_ != nullptr)
        {
            cameraManipulator_->update(deltaTime);
            filament::math::float3 eye, center, up;
            cameraManipulator_->getLookAt(&eye, &center, &up);
            camera_->lookAt(eye, center, up);
        }
    }
#pragma endregion

#pragma region // VzActorRes
    void VzActorRes::SetGeometry(const GeometryVID vid) { vidGeo_ = vid; }
    void VzActorRes::SetMIs(const std::vector<MInstanceVID>& vidMIs)
    {
        vidMIs_ = vidMIs;
    }
    bool VzActorRes::SetMI(const MInstanceVID vid, const int slot)
    {
        if ((size_t)slot >= vidMIs_.size())
        {
            backlog::post("a slot cannot exceed the number of elements in the MI array", backlog::LogLevel::Error);
            return false;
        }
        vidMIs_[slot] = vid;
        return true;
    }
    void VzActorRes::SetMIVariants(const std::vector<std::vector<MInstanceVID>>& vidMIVariants)
    {
        vidMIVariants_ = vidMIVariants;
    }
    GeometryVID VzActorRes::GetGeometryVid() { return vidGeo_; }
    MInstanceVID VzActorRes::GetMIVid(const int slot)
    {
        if ((size_t)slot >= vidMIs_.size())
        {
            backlog::post("a slot cannot exceed the number of elements in the MI array", backlog::LogLevel::Error);
        }
        return vidMIs_[slot];
    }
    std::vector<MInstanceVID>& VzActorRes::GetMIVids()
    {
        return vidMIs_;
    }
    std::vector<std::vector<MInstanceVID>>& VzActorRes::GetMIVariants() { return vidMIVariants_; }
    VzActorRes::~VzActorRes()
    {
        if (isSprite)
        {
            gEngineApp.RemoveComponent(vidMIs_[0]);
            gEngine->destroy(intrinsicVB);
            gEngine->destroy(intrinsicIB);
            gEngine->destroy(intrinsicTexture);
            intrinsicVB = nullptr;
            intrinsicIB = nullptr;
            intrinsicTexture = nullptr;
        }
        assert(intrinsicVB == nullptr && intrinsicIB == nullptr && intrinsicTexture == nullptr);

    }
#pragma endregion

#pragma region // VzLight
#pragma endregion

#pragma region // VzGeometryRes
    VzGeometryRes::~VzGeometryRes()
    {
        // check the ownership
        if (assetOwner == nullptr && !isSystem)
        {
            for (auto& prim : primitives_)
            {
                if (prim.vertices && currentVBs_.find(prim.vertices) != currentVBs_.end()) {
                    gEngine->destroy(prim.vertices);
                    currentVBs_.erase(prim.vertices);
                }
                if (prim.indices && currentIBs_.find(prim.indices) != currentIBs_.end()) {
                    gEngine->destroy(prim.indices);
                    currentIBs_.erase(prim.indices);
                }
                if (prim.morphTargetBuffer && currentMTBs_.find(prim.morphTargetBuffer) != currentMTBs_.end()) {
                    gEngine->destroy(prim.morphTargetBuffer);
                    currentMTBs_.erase(prim.morphTargetBuffer);
                }
            }
        }
    }
    void VzGeometryRes::Set(const std::vector<VzPrimitive>& primitives)
    {
        primitives_ = primitives;
        for (VzPrimitive& prim : primitives_)
        {
            currentVBs_.insert(prim.vertices);
            currentIBs_.insert(prim.indices);
            currentMTBs_.insert(prim.morphTargetBuffer);
        }
    }
    std::vector<VzPrimitive>* VzGeometryRes::Get() { return &primitives_; }

    std::set<VertexBuffer*> VzGeometryRes::currentVBs_;
    std::set<IndexBuffer*> VzGeometryRes::currentIBs_;
    std::set<MorphTargetBuffer*> VzGeometryRes::currentMTBs_;
#pragma endregion

#pragma region // VzMaterialRes
    VzMaterialRes::~VzMaterialRes()
    {
        if (assetOwner == nullptr && !isSystem)
        {
            if (material)
                gEngine->destroy(material);
            material = nullptr;
            // check MI...
        }
    }
#pragma endregion

#pragma region // VzMIRes
    VzMIRes::~VzMIRes()
    {
        if (assetOwner == nullptr && !isSystem)
        {
            if (mi)
                gEngine->destroy(mi);
            mi = nullptr;
            // check MI...
        }
    }
#pragma endregion

#pragma region // VzTextureRes
    VzTextureRes::~VzTextureRes()
    {
        if (assetOwner == nullptr && !isSystem)
        {
            if (texture)
                gEngine->destroy(texture);
            texture = nullptr;
            // check MI...
        }
    }
#pragma endregion
}

#pragma region // VzFontRes
    VzFontRes::~VzFontRes()
    {
        if (ftFace_)
        {
            FT_Done_Face(ftFace_);
            ftFace_ = nullptr;
        }
    }
    bool vzm::VzFontRes::IsSpace(const uint32_t glyphCode)
    {
        return (glyphCode == 0x00000020U) || (glyphCode == 0x00000009U);
    }
    bool vzm::VzFontRes::IsNewLine(const uint32_t glyphCode)
    {
        return (glyphCode == 0x0000000AU);
    }
    int32_t vzm::VzFontRes::GetLineHeight() {
        if (ftFace_) {
            return ftFace_->size->metrics.height >> 6;
        }
        return 0;
    }
    int32_t vzm::VzFontRes::GetBearingX(const uint32_t glyphCode)
    {
        if (ftFace_) {
            renderGlyph(glyphCode);
            return ftFace_->glyph->bitmap_left;
        }
        return 0;
    }
    int32_t vzm::VzFontRes::GetBearingY(const uint32_t glyphCode)
    {
        if (ftFace_) {
            renderGlyph(glyphCode);
            return ftFace_->glyph->bitmap_top;
        }
        return 0;
    }
    int32_t vzm::VzFontRes::GetAdvanceX(const uint32_t glyphCode)
    {
        if (ftFace_) {
            loadGlyph(glyphCode);
            return ftFace_->glyph->advance.x >> 6;
        }
        return 0;
    }
    int32_t vzm::VzFontRes::GetGlyphWidth(const uint32_t glyphCode)
    {
        if (ftFace_) {
            renderGlyph(glyphCode);
            return ftFace_->glyph->bitmap.width;
        }
        return 0;
    }
    int32_t vzm::VzFontRes::GetGlyphHeight(const uint32_t glyphCode)
    {
        if (ftFace_) {
            renderGlyph(glyphCode);
            return ftFace_->glyph->bitmap.rows;
        }
        return 0;
    }
    const uint8_t* vzm::VzFontRes::GetGlyphPixels(const uint32_t glyphCode)
    {
        if (ftFace_) {
            renderGlyph(glyphCode);
            return ftFace_->glyph->bitmap.buffer;
        }
        return nullptr;
    }
    void vzm::VzFontRes::loadGlyph(const uint32_t glyphCode)
    {
        if ((!isLoaded_) || (glyphCode != glyphCode_)) {
            uint32_t glyphIndex = FT_Get_Char_Index(ftFace_, glyphCode);
            FT_Error ft_error = FT_Load_Glyph(ftFace_, glyphIndex, FT_LOAD_DEFAULT);
            if (ft_error)
            {
                backlog::post("Failed to load glyph: " + std::to_string(glyphCode), backlog::LogLevel::Error);
            }
            glyphCode_ = glyphCode;
            isLoaded_ = true;
            isRendered_ = false;
        }
    }
    void vzm::VzFontRes::renderGlyph(const uint32_t glyphCode)
    {
        if ((!isRendered_) || (glyphCode != glyphCode_)) {
            uint32_t glyphIndex = FT_Get_Char_Index(ftFace_, glyphCode);
            FT_Error ft_error = FT_Load_Glyph(ftFace_, glyphIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER);
            if (ft_error)
            {
                backlog::post("Failed to render glyph: " + std::to_string(glyphCode), backlog::LogLevel::Error);
            }
            glyphCode_ = glyphCode;
            isLoaded_ = true;
            isRendered_ = true;
        }
    }
#pragma endregion

namespace vzm
{
    struct GltfIO
    {
        gltfio::VzAssetLoader* assetLoader = nullptr;
        gltfio::VzAssetExpoter* assetExpoter = nullptr;

        gltfio::ResourceLoader* resourceLoader = nullptr;
        gltfio::TextureProvider* stbDecoder = nullptr;
        gltfio::TextureProvider* ktxDecoder = nullptr;

        void Destory()
        {
            if (resourceLoader) {
                resourceLoader->asyncCancelLoad();
                resourceLoader = nullptr;
            }

            delete resourceLoader;
            resourceLoader = nullptr;
            delete stbDecoder;
            stbDecoder = nullptr;
            delete ktxDecoder;
            ktxDecoder = nullptr;

            //AssetLoader::destroy(&assetLoader);
            gltfio::VzAssetLoader::destroy(&assetLoader);
            assetLoader = nullptr;
            assetExpoter = nullptr;
        }
    } vGltfIo;

#pragma region // VzEngineApp
    bool VzEngineApp::removeScene(SceneVID vidScene)
    {
        Scene* scene = GetScene(vidScene);
        if (scene == nullptr)
        {
            return false;
        }
        auto& rcm = gEngine->getRenderableManager();
        auto& lcm = gEngine->getLightManager();
        int retired_ett_count = 0;
        scene->forEach([&](utils::Entity ett) {
            VID vid = ett.getId();
            if (rcm.hasComponent(ett))
            {
                // note that
                // there can be intrinsic entities in the scene
                auto it = actorSceneMap_.find(vid);
                if (it != actorSceneMap_.end()) {
                    it->second = 0;
                    ++retired_ett_count;
                }
            }
            else if (lcm.hasComponent(ett))
            {
                auto it = lightSceneMap_.find(vid);
                if (it != lightSceneMap_.end()) {
                    it->second = 0;
                    ++retired_ett_count;
                }
            }
            else
            {
                // optional.
                auto it = camSceneMap_.find(vid);
                if (it != camSceneMap_.end()) {
                    backlog::post("cam VID : " + std::to_string(ett.getId()), backlog::LogLevel::Default);
                    it->second = 0;
                    ++retired_ett_count;
                }
                else
                {
                    auto& ncm = VzNameCompManager::Get();

                    auto it = actorSceneMap_.find(vid);
                    if (it == actorSceneMap_.end())
                    {
                        backlog::post("entity VID : " + std::to_string(ett.getId()) + " (" + ncm.GetName(ett) + ") ==> not a scene component.. (maybe a bone)", backlog::LogLevel::Warning);
                    }
                    else
                    {
                        backlog::post("entity VID : " + std::to_string(ett.getId()) + " (" + ncm.GetName(ett) + ") is a hierarchy actor (kind of node)", backlog::LogLevel::Default);
                        it->second = 0;
                        ++retired_ett_count;
                    }
                }
            }
            });
        gEngine->destroy(scene);    // maybe.. all views are set to nullptr scenes
        scenes_.erase(vidScene);

        for (auto& it_c : camSceneMap_)
        {
            if (it_c.second == vidScene)
            {
                it_c.second = 0;
                ++retired_ett_count;
            }
        }

        auto& ncm = VzNameCompManager::Get();
        utils::Entity ett = utils::Entity::import(vidScene);
        std::string name = ncm.GetName(ett);
        backlog::post("scene (" + name + ") has been removed, # associated components : " + std::to_string(retired_ett_count),
            backlog::LogLevel::Default);

        auto it_srm = sceneResMap_.find(vidScene);
        assert(it_srm != sceneResMap_.end());
        //scene->remove(it_srm->second.GetLightmapCube()->getSolidRenderable());
        //scene->remove(it_srm->second.GetLightmapCube()->getWireFrameRenderable());
        sceneResMap_.erase(it_srm); // calls destructor

        vzCompMap_.erase(vidScene);
        return true;
    }

    // Runtime can create a new entity with this
    VzScene* VzEngineApp::CreateScene(const std::string& name)
    {
        auto& em = utils::EntityManager::get();
        utils::Entity ett = em.create();
        VID vid = ett.getId();
        scenes_[vid] = gEngine->createScene();
        sceneResMap_[vid] = std::make_unique<VzSceneRes>();

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzScene>(vid, "CreateScene"));
        VzNameCompManager& ncm = VzNameCompManager::Get();
        ncm.CreateNameComp(ett, name);
        return (VzScene*)it.first->second.get();
    }
    VzRenderer* VzEngineApp::CreateRenderPath(const std::string& name)
    {
        // note renderpath involves a renderer
        auto& em = utils::EntityManager::get();
        utils::Entity ett = em.create();
        VID vid = ett.getId();
        renderPathMap_[vid] = std::make_unique<VzRenderPath>();
        VzRenderPath* renderPath = renderPathMap_[vid].get();

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzRenderer>(vid, "CreateRenderPath"));
        VzNameCompManager& ncm = VzNameCompManager::Get();
        ncm.CreateNameComp(ett, name);
        return (VzRenderer*)it.first->second.get();
    }
    VzAsset* VzEngineApp::CreateAsset(const std::string& name)
    {
        auto& em = gEngine->getEntityManager();
        auto& ncm = VzNameCompManager::Get();
        utils::Entity ett = em.create();
        AssetVID vid = ett.getId();
        assetResMap_[vid] = std::make_unique<VzAssetRes>();
        ncm.CreateNameComp(ett, name);

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzAsset>(vid, "CreateAsset"));
        return (VzAsset*)it.first->second.get();
    }
    VzSkeleton* VzEngineApp::CreateSkeleton(const std::string& name, const SkeletonVID vidExist)
    {
        auto& em = gEngine->getEntityManager();
        auto& ncm = VzNameCompManager::Get();
        utils::Entity ett = utils::Entity::import(vidExist);
        if (ett.isNull()) {
            ett = em.create();
        }

        AssetVID vid = ett.getId();
        skeletonResMap_[vid] = std::make_unique<VzSkeletonRes>();
        ncm.CreateNameComp(ett, name);

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzAsset>(vid, "CreateSkeleton"));
        return (VzSkeleton*)it.first->second.get();
    }
    size_t VzEngineApp::GetVidsByName(const std::string& name, std::vector<VID>& vids)
    {
        VzNameCompManager& ncm = VzNameCompManager::Get();
        std::vector<utils::Entity> etts = ncm.GetEntitiesByName(name);
        size_t num_etts = etts.size();
        if (num_etts == 0)
        {
            return 0u;
        }

        vids.clear();
        vids.reserve(num_etts);
        for (utils::Entity& ett : etts)
        {
            vids.push_back(ett.getId());
        }
        return num_etts;
    }
    VID VzEngineApp::GetFirstVidByName(const std::string& name)
    {
        VzNameCompManager& ncm = VzNameCompManager::Get();
        utils::Entity ett = ncm.GetFirstEntityByName(name);
        //if (ett.isNull())
        //{
        //    return INVALID_VID;
        //}
        return ett.getId();
    }
    std::string VzEngineApp::GetNameByVid(const VID vid)
    {
        VzNameCompManager& ncm = VzNameCompManager::Get();
        return ncm.GetName(utils::Entity::import(vid));
    }
    bool VzEngineApp::HasComponent(const VID vid)
    {
        VzNameCompManager& ncm = VzNameCompManager::Get();
        return ncm.hasComponent(utils::Entity::import(vid));
    }
    bool VzEngineApp::IsRenderable(const ActorVID vid)
    {
        bool ret = gEngine->getRenderableManager().hasComponent(utils::Entity::import(vid));
#ifdef _DEBUG
        auto it = actorSceneMap_.find(vid);
        assert(ret == (it != actorSceneMap_.end()));
#endif
        return ret;
    }
    bool VzEngineApp::IsSceneComponent(VID vid) // can be a node for a scene tree
    {
        return scenes_.contains(vid) || camSceneMap_.contains(vid) || lightSceneMap_.contains(vid) || actorSceneMap_.contains(vid);
    }
    bool VzEngineApp::IsLight(const LightVID vid)
    {
        bool ret = gEngine->getLightManager().hasComponent(utils::Entity::import(vid));
#ifdef _DEBUG
        auto it = lightSceneMap_.find(vid);
        assert(ret == (it != lightSceneMap_.end()));
#endif
        return ret;
    }
    Scene* VzEngineApp::GetScene(const SceneVID vid)
    {
        auto it = scenes_.find(vid);
        if (it == scenes_.end())
        {
            return nullptr;
        }
        return it->second;
    }
    Scene* VzEngineApp::GetFirstSceneByName(const std::string& name)
    {
        VzNameCompManager& ncm = VzNameCompManager::Get();
        std::vector<utils::Entity> etts = ncm.GetEntitiesByName(name);
        if (etts.size() == 0)
        {
            return nullptr;
        }

        for (utils::Entity& ett : etts)
        {
            SceneVID sid = ett.getId();
            auto it = scenes_.find(sid);
            if (it != scenes_.end())
            {
                return it->second;
            }
        }
        return nullptr;
    }
    std::unordered_map<SceneVID, Scene*>* VzEngineApp::GetScenes()
    {
        return &scenes_;
    }

#define GET_RES_PTR(RES_MAP) auto it = RES_MAP.find(vid); if (it == RES_MAP.end()) return nullptr; return it->second.get();
    VzSceneRes* VzEngineApp::GetSceneRes(const SceneVID vid)
    {
        GET_RES_PTR(sceneResMap_);
    }
    VzRenderPath* VzEngineApp::GetRenderPath(const RendererVID vid)
    {
        GET_RES_PTR(renderPathMap_);
    }
    VzCameraRes* VzEngineApp::GetCameraRes(const CamVID vid)
    {
        GET_RES_PTR(camResMap_);
    }
    VzActorRes* VzEngineApp::GetActorRes(const ActorVID vid)
    {
        GET_RES_PTR(actorResMap_);
    }
    VzLightRes* VzEngineApp::GetLightRes(const LightVID vid)
    {
        GET_RES_PTR(lightResMap_);
    }
    VzAssetRes* VzEngineApp::GetAssetRes(const AssetVID vid)
    {
        GET_RES_PTR(assetResMap_);
    }
    VzSkeletonRes* VzEngineApp::GetSkeletonRes(const SkeletonVID vid)
    {
        GET_RES_PTR(skeletonResMap_);
    }
    AssetVID VzEngineApp::GetAssetOwner(VID vid)
    {
        for (auto& it : assetResMap_)
        {
            VzAssetRes& asset_res = *it.second.get();
            if (asset_res.assetOwnershipComponents.contains(vid))
            {
                return it.first;
            }
        }
        return INVALID_VID;
    }

    size_t VzEngineApp::GetCameraVids(std::vector<CamVID>& camVids)
    {
        camVids.clear();
        camVids.reserve(camSceneMap_.size());
        for (auto& it : camSceneMap_)
        {
            camVids.push_back(it.first);
        }
        return camVids.size();
    }
    size_t VzEngineApp::GetActorVids(std::vector<ActorVID>& actorVids)
    {
        actorVids.clear();
        actorVids.reserve(actorSceneMap_.size());
        for (auto& it : actorSceneMap_)
        {
            actorVids.push_back(it.first);
        }
        return actorVids.size();
    }
    size_t VzEngineApp::GetLightVids(std::vector<LightVID>& lightVids)
    {
        lightVids.clear();
        lightVids.reserve(lightSceneMap_.size());
        for (auto& it : lightSceneMap_)
        {
            lightVids.push_back(it.first);
        }
        return lightVids.size();
    }
    size_t VzEngineApp::GetRenderPathVids(std::vector<RendererVID>& renderPathVids)
    {
        renderPathVids.clear();
        renderPathVids.reserve(renderPathMap_.size());
        for (auto& it : renderPathMap_)
        {
            renderPathVids.push_back(it.first);
        }
        return renderPathVids.size();
    }
    VzRenderPath* VzEngineApp::GetFirstRenderPathByName(const std::string& name)
    {
        return GetRenderPath(GetFirstVidByName(name));
    }
    SceneVID VzEngineApp::GetSceneVidBelongTo(const VID vid)
    {
        auto itr = actorSceneMap_.find(vid);
        if (itr != actorSceneMap_.end())
        {
            return itr->second;
        }
        auto itl = lightSceneMap_.find(vid);
        if (itl != lightSceneMap_.end())
        {
            return itl->second;
        }
        auto itc = camSceneMap_.find(vid);
        if (itc != camSceneMap_.end())
        {
            return itc->second;
        }
        return INVALID_VID;
    }
    size_t VzEngineApp::GetSceneCompChildren(const SceneVID vidScene, std::vector<VID>& vidChildren)
    {
        vidChildren.clear();
        auto& tcm = gEngine->getTransformManager();
        for (auto& it : actorSceneMap_)
        {
            if (it.second == vidScene)
            {
                auto ins = tcm.getInstance(Entity::import(it.first));
                if (tcm.getParent(ins).isNull())
                {
                    vidChildren.push_back(it.first);
                }
            }
        }
        for (auto& it : lightSceneMap_)
        {
            if (it.second == vidScene)
            {
                auto ins = tcm.getInstance(Entity::import(it.first));
                if (tcm.getParent(ins).isNull())
                {
                    vidChildren.push_back(it.first);
                }
            }
        }
        for (auto& it : camSceneMap_)
        {
            if (it.second == vidScene)
            {
                auto ins = tcm.getInstance(Entity::import(it.first));
                if (tcm.getParent(ins).isNull())
                {
                    vidChildren.push_back(it.first);
                }
            }
        }
        return vidChildren.size();
    }

    bool VzEngineApp::AppendSceneEntityToParent(const VID vidSrc, const VID vidDst)
    {
        assert(vidSrc != vidDst);
        auto& tcm = gEngine->getTransformManager();

        auto getSceneAndVid = [this](Scene** scene, const VID vid)
            {
                SceneVID vid_scene = vid;
                *scene = GetScene(vid_scene);
                if (*scene == nullptr)
                {
                    auto itr = actorSceneMap_.find(vid);
                    auto itl = lightSceneMap_.find(vid);
                    auto itc = camSceneMap_.find(vid);
                    if (itr == actorSceneMap_.end()
                        && itl == lightSceneMap_.end()
                        && itc == camSceneMap_.end())
                    {
                        vid_scene = INVALID_VID;
                    }
                    else
                    {
                        vid_scene = max(max(itl != lightSceneMap_.end() ? itl->second : INVALID_VID,
                            itr != actorSceneMap_.end() ? itr->second : INVALID_VID),
                            itc != camSceneMap_.end() ? itc->second : INVALID_VID);
                        //assert(vid_scene != INVALID_VID); can be INVALID_VID
                        *scene = GetScene(vid_scene);
                    }
                }
                return vid_scene;
            };

        Scene* scene_src = nullptr;
        Scene* scene_dst = nullptr;
        SceneVID vid_scene_src = getSceneAndVid(&scene_src, vidSrc);
        SceneVID vid_scene_dst = getSceneAndVid(&scene_dst, vidDst);

        utils::Entity ett_src = utils::Entity::import(vidSrc);
        utils::Entity ett_dst = utils::Entity::import(vidDst);
        //auto& em = gEngine->getEntityManager();

        // case 1. both entities are actor
        // case 2. src is scene and dst is actor
        // case 3. src is actor and dst is scene
        // case 4. both entities are scenes
        // note that actor entity must have transform component!
        std::vector<utils::Entity> entities_moving;
        if (vidSrc != vid_scene_src && vidDst != vid_scene_dst)
        {
            // case 1. both entities are actor
            auto ins_src = tcm.getInstance(ett_src);
            auto ins_dst = tcm.getInstance(ett_dst);
            assert(ins_src.asValue() != 0 && ins_dst.asValue() != 0);

            tcm.setParent(ins_src, ins_dst);

            entities_moving.push_back(ett_src);
            getDescendants(ett_src, entities_moving);
            //for (auto it = tcm.getChildrenBegin(ins_src); it != tcm.getChildrenEnd(ins_src); it++)
            //{
            //    utils::Entity ett = tcm.getEntity(*it);
            //    entities_moving.push_back(ett);
            //}
        }
        else if (vidSrc == vid_scene_src && vidDst != vid_scene_dst)
        {
            assert(scene_src != scene_dst && "scene cannot be appended to its component");

            // case 2. src is scene and dst is actor
            auto ins_dst = tcm.getInstance(ett_dst);
            assert(ins_dst.asValue() != 0 && "vidDst is invalid");
            scene_src->forEach([&](utils::Entity ett) {
                entities_moving.push_back(ett);

                auto ins = tcm.getInstance(ett);
                utils::Entity ett_parent = tcm.getParent(ins);
                if (ett_parent.isNull())
                {
                    tcm.setParent(ins, ins_dst);
                }
                });
        }
        else if (vidSrc != vid_scene_src && vidDst == vid_scene_dst)
        {
            // case 3. src is actor and dst is scene
            // scene_src == scene_dst means that 
            //    vidSrc is appended to its root

            auto ins_src = tcm.getInstance(ett_src);
            assert(ins_src.asValue() != 0 && "vidSrc is invalid");

            entities_moving.push_back(ett_src);
            getDescendants(ett_src, entities_moving);
            //for (auto it = tcm.getChildrenBegin(ins_src); it != tcm.getChildrenEnd(ins_src); it++)
            //{
            //    utils::Entity ett = tcm.getEntity(*it);
            //    entities_moving.push_back(ett);
            //}
        }
        else
        {
            assert(vidSrc == vid_scene_src && vidDst == vid_scene_dst);
            assert(scene_src != scene_dst);
            if (scene_src == nullptr)
            {
                return false;
            }
            // case 4. both entities are scenes
            scene_src->forEach([&](utils::Entity ett) {
                entities_moving.push_back(ett);
                });

            removeScene(vid_scene_src);
            scene_src = nullptr;
        }

        // NOTE 
        // a scene can have entities with components that are not renderable
        // they are supposed to be ignored during the rendering pipeline

        for (auto& it : entities_moving)
        {
            auto itr = actorSceneMap_.find(it.getId());
            auto itl = lightSceneMap_.find(it.getId());
            auto itc = camSceneMap_.find(it.getId());
            if (itr != actorSceneMap_.end())
                itr->second = 0;
            else if (itl != lightSceneMap_.end())
                itl->second = 0;
            else if (itc != camSceneMap_.end())
                itc->second = 0;
            if (scene_src)
            {
                scene_src->remove(ett_src);
            }
        }

        if (scene_dst)
        {
            for (auto& it : entities_moving)
            {
                // The entity is ignored if it doesn't have a Renderable or Light component.
                scene_dst->addEntity(it);

                auto itr = actorSceneMap_.find(it.getId());
                auto itl = lightSceneMap_.find(it.getId());
                auto itc = camSceneMap_.find(it.getId());
                if (itr != actorSceneMap_.end())
                    itr->second = vid_scene_dst;
                if (itl != lightSceneMap_.end())
                    itl->second = vid_scene_dst;
                if (itc != camSceneMap_.end())
                    itc->second = vid_scene_dst;
            }
        }
        return true;
    }

    VzSceneComp* VzEngineApp::CreateSceneComponent(const SCENE_COMPONENT_TYPE compType, const std::string& name, const VID vidExist)
    {
        if (compType == SCENE_COMPONENT_TYPE::SCENEBASE)
        {
            return nullptr;
        }

        VID vid = vidExist;
        auto& em = gEngine->getEntityManager();
        utils::Entity ett = utils::Entity::import(vid);
        bool is_alive = em.isAlive(ett);
        if (!is_alive)
        {
            ett = em.create();
            vid = ett.getId();
        }

        auto& ncm = VzNameCompManager::Get();
        auto& tcm = gEngine->getTransformManager();

        if (!ncm.getInstance(ett).isValid())
        {
            ncm.CreateNameComp(ett, name);
        }
        if (!tcm.getInstance(ett).isValid())
        {
            tcm.create(ett);
        }

        VzSceneComp* v_comp = nullptr;

        switch (compType)
        {
        case SCENE_COMPONENT_TYPE::SPRITE_ACTOR:
        case SCENE_COMPONENT_TYPE::TEXT_SPRITE_ACTOR:
        {
            actorSceneMap_[vid] = 0; // first creation
            actorResMap_[vid] = std::make_unique<VzActorRes>();

            auto it = compType == SCENE_COMPONENT_TYPE::SPRITE_ACTOR?
                vzCompMap_.emplace(vid, std::make_unique<VzSpriteActor>(vid, "CreateSceneComponent")) :
                vzCompMap_.emplace(vid, std::make_unique<VzTextSpriteActor>(vid, "CreateSceneComponent"));
            v_comp = (VzSceneComp*)it.first->second.get();

            VzActorRes* actor_res = actorResMap_[vid].get();
            actor_res->isSprite = true;

            actor_res->intrinsicCache.assign(80 + 12, 0);

            MaterialVID vid_m = INVALID_VID;
            if (compType == SCENE_COMPONENT_TYPE::SPRITE_ACTOR)
            {
                vid_m = GetFirstVidByName("_PROVIDER_SPRITE_MATERIAL");
                if (vid_m == INVALID_VID)
                {
                    MaterialKey m_key = {};
                    m_key.alphaMode = AlphaMode::BLEND;
                    m_key.doubleSided = true;
                    m_key.hasBaseColorTexture = true;
                    m_key.unlit = true;
                    m_key.baseColorUV = 0;
                    UvMap uvmap;
                    Material* material = gMaterialProvider->getMaterial((filament::gltfio::MaterialKey*)&m_key, &uvmap, "_PROVIDER_SPRITE_MATERIAL");
                    vid_m = gEngineApp.CreateMaterial("_PROVIDER_SPRITE_MATERIAL", material, nullptr, true)->GetVID();
                    std::vector<Material::ParameterInfo> params(material->getParameterCount());
                    material->getParameters(&params[0], params.size());
                    for (auto& it : params)
                    {
                        std::cout << it.name << ", " << (uint8_t)it.type << std::endl;
                    }
                }
            }
            else
            {
                vid_m = GetFirstVidByName("_TEXT_SPRITE_MATERIAL");
                if (vid_m == INVALID_VID) {
                    MaterialBuilder::init();
                    const char* code = R"(
                        void material(inout MaterialInputs material) {
                            prepareMaterial(material);
                            material.baseColor.rgb = materialParams.textColor;
                            material.baseColor.a = texture(materialParams_textTexture, getUV0()).r;
                        }
                    )";
                    MaterialBuilder builder;
                    builder
                        .name("TextSpriteMaterial")
                        .shading(Shading::UNLIT)
                        .blending(BlendingMode::TRANSPARENT)
                        .parameter("textTexture",
                                   (MaterialBuilder::SamplerType) SamplerType::SAMPLER_2D,
                                   SamplerFormat::FLOAT,
                                   (MaterialBuilder::Precision) Precision::MEDIUM)
                        .parameter("textColor",
                                   (MaterialBuilder::UniformType) UniformType::FLOAT3,
                                   (MaterialBuilder::Precision) Precision::MEDIUM)
                        .require(MaterialBuilder::VertexAttribute::UV0)
                        .doubleSided(true)
                        .flipUV(false)
                        .material(code);
                    Package result = builder.build(gEngine->getJobSystem());
                    assert(result.isValid());
                    Material* material = Material::Builder()
                        .package(result.getData(), result.getSize())
                        .build(*gEngine);
                    vid_m = gEngineApp.CreateMaterial("_TEXT_SPRITE_MATERIAL", material, nullptr, true)->GetVID();
                    std::vector<Material::ParameterInfo> params(material->getParameterCount());
                    material->getParameters(&params[0], params.size());
                    for (auto& it : params) {
                        std::cout << it.name << ", " << (uint8_t) it.type << std::endl;
                    }
                }
            }

            Material* m = materialResMap_[vid_m]->material;
            MaterialInstance* mi = m->createInstance();

            if (compType == SCENE_COMPONENT_TYPE::SPRITE_ACTOR)
            {
                mi->setParameter("baseColorFactor", filament::RgbaType::LINEAR, filament::math::float4{1.0, 1.0, 1.0, 1.0});
            }
            mi->setDoubleSided(true);
            VzMI* v_mi = CreateMaterialInstance(name + "_mi", vid_m, mi);
            actor_res->SetMIs({ v_mi->GetVID() });
            actor_res->culling = false;
            actor_res->castShadow = false;
            actor_res->receiveShadow = false;

            ((VzSpriteActor*)v_comp)->SetGeometry();

            break;
        }
        case SCENE_COMPONENT_TYPE::ACTOR:
        {
            // RenderableManager::Builder... with entity registers the entity in the renderableEntities
            actorSceneMap_[vid] = 0; // first creation
            actorResMap_[vid] = std::make_unique<VzActorRes>();

            auto it = vzCompMap_.emplace(vid, std::make_unique<VzActor>(vid, "CreateSceneComponent"));
            v_comp = (VzSceneComp*)it.first->second.get();
            break;
        }
        case SCENE_COMPONENT_TYPE::LIGHT:
        {
            if (!is_alive)
            {
                LightManager::Builder(LightManager::Type::SUN)
                    .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                    .intensity(110000)
                    .direction({ 0.7, -1, -0.8 })
                    .sunAngularRadius(1.9f)
                    .castShadows(false)
                    .build(*gEngine, ett);
            }
            lightSceneMap_[vid] = 0; // first creation
            lightResMap_[vid] = std::make_unique<VzLightRes>();

            auto it = vzCompMap_.emplace(vid, std::make_unique<VzLight>(vid, "CreateSceneComponent"));
            v_comp = (VzSceneComp*)it.first->second.get();
            break;
        }
        case SCENE_COMPONENT_TYPE::CAMERA:
        {
            Camera* camera = nullptr;
            if (!is_alive)
            {
                camera = gEngine->createCamera(ett);
                camera->setExposure(16.0f, 1 / 125.0f, 100.0f); // default values used in filamentApp
            }
            else
            {
                camera = gEngine->getCameraComponent(ett);
            }
            camSceneMap_[vid] = 0;
            camResMap_[vid] = std::make_unique<VzCameraRes>();
            VzCameraRes* cam_res = camResMap_[vid].get();
            cam_res->SetCamera(camera);

            auto it = vzCompMap_.emplace(vid, std::make_unique<VzCamera>(vid, "CreateSceneComponent"));
            v_comp = (VzSceneComp*)it.first->second.get();
            v_comp->SetMatrixAutoUpdate(false);
            break;
        }
        default:
            assert(0);
        }

        mat4f localTransform = tcm.getTransform(tcm.getInstance(ett));
        float3 scale;
        quatf rotation;
        float3 translation;
        decomposeMatrix(localTransform, &translation, &rotation, &scale);

        v_comp->SetScale(__FP(scale));
        v_comp->SetQuaternion(__FP(rotation));
        v_comp->SetPosition(__FP(translation));

        return v_comp;
    }
    VzActor* VzEngineApp::CreateTestActor(const std::string& modelName)
    {
        std::string geo_name = modelName + "_GEOMETRY";
        const std::string material_name = "_DEFAULT_STANDARD_MATERIAL";
        const std::string mi_name = modelName + "__MI";
        auto& ncm = VzNameCompManager::Get();

        MaterialInstance* mi = nullptr;
        MInstanceVID vid_mi = INVALID_VID;
        for (auto& it_mi : miResMap_)
        {
            utils::Entity ett = utils::Entity::import(it_mi.first);

            if (ncm.GetName(ett) == mi_name)
            {
                mi = it_mi.second->mi;
                vid_mi = it_mi.first;
                break;
            }
        }
        if (mi == nullptr)
        {
            MaterialVID vid_m = GetFirstVidByName(material_name);
            VzMaterial* v_m = GetVzComponent<VzMaterial>(vid_m);
            assert(v_m != nullptr);
            Material* m = materialResMap_[v_m->GetVID()]->material;
            mi = m->createInstance(mi_name.c_str());
            mi->setParameter("baseColor", filament::RgbaType::LINEAR, filament::math::float4{ 0.8, 0.1, 0.1, 1.0 });
            mi->setParameter("metallic", 1.0f);
            mi->setParameter("roughness", 0.4f);
            mi->setParameter("reflectance", 0.5f);
            VzMI* v_mi = CreateMaterialInstance(mi_name, vid_m, mi);
            auto it_mi = miResMap_.find(v_mi->GetVID());
            assert(it_mi != miResMap_.end());
            assert(mi == it_mi->second->mi);
            vid_mi = v_mi->GetVID();
        }
        assert(vid_mi != INVALID_VID);

        MeshReader::Mesh mesh = MeshReader::loadMeshFromBuffer(gEngine, MONKEY_SUZANNE_DATA, nullptr, nullptr, mi);
        ncm.CreateNameComp(mesh.renderable, modelName);
        VID vid = mesh.renderable.getId();
        actorSceneMap_[vid] = 0;
        actorResMap_[vid] = std::make_unique<VzActorRes>();

        auto& rcm = gEngine->getRenderableManager();
        auto ins = rcm.getInstance(mesh.renderable);
        Box box = rcm.getAxisAlignedBoundingBox(ins);

        VzGeometry* geo = CreateGeometry(geo_name, std::vector<VzPrimitive>{VzPrimitive{
            .vertices = mesh.vertexBuffer,
            .indices = mesh.indexBuffer,
            .aabb = {.min = box.getMin(), .max = box.getMax() },
            } });
        VzActorRes& actor_res = *actorResMap_[vid].get();
        actor_res.SetGeometry(geo->GetVID());
        actor_res.SetMIs({ vid_mi });

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzActor>(vid, "CreateTestActor"));
        VzActor* v_actor = (VzActor*)it.first->second.get();
        return v_actor;
    }
    VzGeometry* VzEngineApp::CreateGeometry(const std::string& name,
        const std::vector<VzPrimitive>& primitives,
        const filament::gltfio::FilamentAsset* assetOwner,
        const bool isSystem)
    {
        auto& em = utils::EntityManager::get();
        auto& ncm = VzNameCompManager::Get();

        utils::Entity ett = em.create();
        ncm.CreateNameComp(ett, name);

        VID vid = ett.getId();

        geometryResMap_[vid] = std::make_unique<VzGeometryRes>();
        VzGeometryRes& geo_res = *geometryResMap_[vid].get();
        geo_res.Set(primitives);
        geo_res.assetOwner = (filament::gltfio::FilamentAsset*)assetOwner;
        geo_res.isSystem = isSystem;
        for (auto& prim : primitives)
        {
            geo_res.aabb.min = min(prim.aabb.min, geo_res.aabb.min);
            geo_res.aabb.max = max(prim.aabb.max, geo_res.aabb.max);
        }

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzGeometry>(vid, "CreateGeometry"));
        return (VzGeometry*)it.first->second.get();;
    }
    VzMaterial* VzEngineApp::CreateMaterial(const std::string& name,
        const Material* material,
        const filament::gltfio::FilamentAsset* assetOwner,
        const bool isSystem)
    {
        auto& em = utils::EntityManager::get();
        auto& ncm = VzNameCompManager::Get();

        if (material != nullptr)
        {
            for (auto& it : materialResMap_)
            {
                if (it.second->material == material)
                {
                    backlog::post("The material has already been registered!", backlog::LogLevel::Warning);
                }
            }
        }

        utils::Entity ett = em.create();
        ncm.CreateNameComp(ett, name);

        VID vid = ett.getId();
        materialResMap_[vid] = std::make_unique<VzMaterialRes>();
        VzMaterialRes& m_res = *materialResMap_[vid].get();
        m_res.material = (Material*)material;
        m_res.assetOwner = (filament::gltfio::FilamentAsset*)assetOwner;
        m_res.isSystem = isSystem;

        if (material != nullptr) {        
            std::vector<Material::ParameterInfo> params(material->getParameterCount());
            material->getParameters(&params[0], params.size());
            for (auto& param : params)
            {
                assert(!m_res.allowedParamters.contains(param.name));
                m_res.allowedParamters[param.name] = param;
            }
        }
        auto it = vzCompMap_.emplace(vid, std::make_unique<VzMaterial>(vid, "CreateMaterial"));
        return (VzMaterial*)it.first->second.get();
    }
    VzMI* VzEngineApp::CreateMaterialInstance(const std::string& name,
        const MaterialVID vidMaterial,
        const MaterialInstance* mi,
        const filament::gltfio::FilamentAsset* assetOwner,
        const bool isSystem)
    {
        auto& em = utils::EntityManager::get();
        auto& ncm = VzNameCompManager::Get();

        if (mi != nullptr)
        {
            for (auto& it : miResMap_)
            {
                if (it.second->mi == mi)
                {
                    backlog::post("The material instance has already been registered!", backlog::LogLevel::Warning);
                }
            }
        }

        utils::Entity ett = em.create();
        ncm.CreateNameComp(ett, name);

        VID vid = ett.getId();
        miResMap_[vid] = std::make_unique<VzMIRes>();
        VzMIRes& mi_res = *miResMap_[vid].get();
        mi_res.vidMaterial = vidMaterial;
        mi_res.mi = (MaterialInstance*)mi;
        mi_res.assetOwner = (filament::gltfio::FilamentAsset*)assetOwner;
        mi_res.isSystem = isSystem;

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzMI>(vid, "CreateMaterialInstance"));
        return (VzMI*)it.first->second.get();
    }
    VzTexture* VzEngineApp::CreateTexture(const std::string& name,
        const Texture* texture,
        const filament::gltfio::FilamentAsset* assetOwner,
        const bool isSystem)
    {
        auto& em = utils::EntityManager::get();
        auto& ncm = VzNameCompManager::Get();

        utils::Entity ett = em.create();
        ncm.CreateNameComp(ett, name);

        VID vid = ett.getId();
        textureResMap_[vid] = std::make_unique<VzTextureRes>();
        VzTextureRes& tex_res = *textureResMap_[vid].get();
        tex_res.texture = (Texture*)texture;
        tex_res.assetOwner = (filament::gltfio::FilamentAsset*)assetOwner;
        tex_res.isSystem = isSystem;

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzTexture>(vid, "CreateTexture"));
        return (VzTexture*)it.first->second.get();
    }
    VzFont* VzEngineApp::CreateFont(const std::string& name)
    {
        auto& em = utils::EntityManager::get();
        auto& ncm = VzNameCompManager::Get();

        utils::Entity ett = em.create();
        ncm.CreateNameComp(ett, name);

        VID vid = ett.getId();
        fontResMap_[vid] = std::make_unique<VzFontRes>();

        auto it = vzCompMap_.emplace(vid, std::make_unique<VzFont>(vid, "CreateFont"));
        return (VzFont*)it.first->second.get();
    }

    void VzEngineApp::BuildRenderable(const ActorVID vid)
    {
        VzActorRes* actor_res = GetActorRes(vid);
        if (actor_res == nullptr)
        {
            backlog::post("invalid ActorVID", backlog::LogLevel::Error);
            return;
        }
        VzGeometryRes* geo_res = GetGeometryRes(actor_res->GetGeometryVid());
        if (geo_res == nullptr)
        {
            backlog::post("invalid GetGeometryVid", backlog::LogLevel::Error);
            return;
        }

        //auto ins = rcm.getInstance(ett_actor);
        //if (ins.isValid())
        //{
        //    ett_actor = gEngine->getEntityManager().create();
        //    ins = rcm.getInstance(ett_actor);\
        //}
        //assert(ins.isValid());

        std::vector<VzPrimitive>& primitives = *geo_res->Get();
        std::vector<MaterialInstance*> mis;
        for (auto& it_mi : actor_res->GetMIVids())
        {
            mis.push_back(GetMIRes(it_mi)->mi);
        }

        RenderableManager::Builder builder(primitives.size());

        for (size_t index = 0, n = primitives.size(); index < n; ++index)
        {
            VzPrimitive* primitive = &primitives[index];
            MaterialInstance* mi = mis.size() > index ? mis[index] : nullptr;
            RenderableManager::PrimitiveType prim_type = (RenderableManager::PrimitiveType)primitive->ptype;
            builder.material(index, mi);
            builder.geometry(index, prim_type, primitive->vertices, primitive->indices);
            if (primitive->morphTargetBuffer)
            {
                builder.morphing(primitive->morphTargetBuffer);
            }
        }

        utils::Entity ett_actor = utils::Entity::import(vid);

        std::string name = VzNameCompManager::Get().GetName(ett_actor);
        Box box = Box().set(geo_res->aabb.min, geo_res->aabb.max);
        if (box.isEmpty()) {
            backlog::post("Missing bounding box in " + name, backlog::LogLevel::Warning);
            box = Box().set(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
        }

        builder
            .boundingBox(box)
            .culling(actor_res->culling)
            .castShadows(actor_res->castShadow)
            .receiveShadows(actor_res->receiveShadow)
            .build(*gEngine, ett_actor);
    }

    VzGeometryRes* VzEngineApp::GetGeometryRes(const GeometryVID vidGeo)
    {
        auto it = geometryResMap_.find(vidGeo);
        if (it == geometryResMap_.end())
        {
            return nullptr;
        }
        return it->second.get();
    }
    VzMaterialRes* VzEngineApp::GetMaterialRes(const MaterialVID vidMaterial)
    {
        auto it = materialResMap_.find(vidMaterial);
        if (it == materialResMap_.end())
        {
            return nullptr;
        }
        return it->second.get();
    }
    MaterialVID VzEngineApp::FindMaterialVID(const filament::Material* mat)
    {
        for (auto& it : materialResMap_)
        {
            if (it.second->material == mat)
            {
                return it.first;
            }
        }
        return INVALID_VID;
    }

    VzMIRes* VzEngineApp::GetMIRes(const MInstanceVID vidMI)
    {
        auto it = miResMap_.find(vidMI);
        if (it == miResMap_.end())
        {
            return nullptr;
        }
        return it->second.get();
    }
    MInstanceVID VzEngineApp::FindMaterialInstanceVID(const filament::MaterialInstance* mi)
    {
        for (auto& it : miResMap_)
        {
            if (it.second->mi == mi)
            {
                return it.first;
            }
        }
        return INVALID_VID;
    }
    VzTextureRes* VzEngineApp::GetTextureRes(const TextureVID vidTex)
    {
        auto it = textureResMap_.find(vidTex);
        if (it == textureResMap_.end())
        {
            return nullptr;
        }
        return it->second.get();
    }
    TextureVID VzEngineApp::FindTextureVID(const filament::Texture* texture)
    {
        for (auto& it : textureResMap_)
        {
            if (it.second->texture == texture)
            {
                return it.first;
            }
        }
        return INVALID_VID;
    }
    VzFontRes* VzEngineApp::GetFontRes(const FontVID vidFont)
    {
        auto it = fontResMap_.find(vidFont);
        if (it == fontResMap_.end())
        {
            return nullptr;
        }
        return it->second.get();
    }

    size_t VzEngineApp::LoadMeshFile(const std::string& filename, std::vector<VzActor*>& actors)
    {
        // Add geometry into the scene.
        assimp::VzMeshAssimp* meshes = new assimp::VzMeshAssimp(*gEngine);
        
        std::vector<ActorVID> loaded_actors;
        meshes->addFromFile(filename, loaded_actors);
        actors.clear();
        for (size_t i = 0, n = loaded_actors.size(); i < n; ++i)
        {
            actors.push_back(gEngineApp.GetVzComponent<VzActor>(loaded_actors[i]));
        }
        delete meshes;
        return loaded_actors.size();
    }

    VzAssetLoader* VzEngineApp::GetGltfAssetLoader()
    {
        return vGltfIo.assetLoader;
    }
    
    VzAssetExpoter* VzEngineApp::GetGltfAssetExpoter()
    {
        return vGltfIo.assetExpoter;
    }

    ResourceLoader* VzEngineApp::GetGltfResourceLoader()
    {
        return vGltfIo.resourceLoader;
    }

    bool VzEngineApp::RemoveComponent(const VID vid, const bool ignoreOnwership)
    {
        utils::Entity ett = utils::Entity::import(vid);

        VzBaseComp* b = GetVzComponent<VzBaseComp>(vid);
        if (b == nullptr)
        {
            return false;
        }

        auto& em = utils::EntityManager::get();
        auto& ncm = VzNameCompManager::Get();
        std::string name = ncm.GetName(ett);
        if (!removeScene(vid))
        {
            // this calls built-in destroy functions in the filament entity managers

            // destroy by engine (refer to the following)
            // void FEngine::destroy(Entity e) {
            //     mRenderableManager.destroy(e);
            //     mLightManager.destroy(e);
            //     mTransformManager.destroy(e);
            //     mCameraManager.destroy(*this, e);
            // }
#pragma region destroy by engine
            bool isRenderableResource = false; // geometry, material, or material instance
            // note filament engine handles the renderable objects (with the modified resource settings)
            auto it_tx = textureResMap_.find(vid);
            if (it_tx != textureResMap_.end())
            {
                VzTextureRes& tex_res = *it_tx->second.get();
                if (tex_res.isSystem && !ignoreOnwership)
                {
                    backlog::post("Texture (" + name + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else if (tex_res.assetOwner && !ignoreOnwership)
                {
                    AssetVID vid_asset = GetAssetOwner(it_tx->first);
                    utils::Entity ett_asset = utils::Entity::import(vid_asset);
                    std::string name_asset = ncm.GetName(ett_asset);
                    assert(vid_asset);
                    backlog::post("Texture (" + name + ") is asset(" + name_asset + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else if (tex_res.isAsyncLocked)
                {
                    backlog::post("Texture (" + name + ") is under asynchronous loading, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else
                {
                    for (auto& it : miResMap_)
                    {
                        VzMIRes* mi_res = it.second.get();
                        std::vector<std::string> delete_keys;
                        for (auto& tex_map_kv : mi_res->texMap)
                        {
                            if (tex_map_kv.second == it_tx->first)
                            {
                                delete_keys.push_back(tex_map_kv.first);
                            }
                        }
                        for (auto& del_key : delete_keys)
                        {
                            mi_res->texMap.erase(del_key);
                        }
                    }

                    textureResMap_.erase(it_tx); // call destructor...
                    isRenderableResource = true;
                    backlog::post("Texture (" + name + ") has been removed", backlog::LogLevel::Default);
                }
            }
            auto it_m = materialResMap_.find(vid);
            if (it_m != materialResMap_.end())
            {
                VzMaterialRes& m_res = *it_m->second.get();
                if (m_res.isSystem && !ignoreOnwership)
                {
                    backlog::post("Material (" + name + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else if (m_res.assetOwner && !ignoreOnwership)
                {
                    AssetVID vid_asset = GetAssetOwner(it_m->first);
                    utils::Entity ett_asset = utils::Entity::import(vid_asset);
                    std::string name_asset = ncm.GetName(ett_asset);
                    assert(vid_asset);
                    backlog::post("Material (" + name + ") is asset(" + name_asset + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else
                {
                    for (auto it = miResMap_.begin(); it != miResMap_.end();)
                    {
                        if (it->second->mi->getMaterial() == m_res.material)
                        {
                            utils::Entity ett_mi = utils::Entity::import(it->first);
                            std::string name_mi = ncm.GetName(ett_mi);
                            if (it->second->isSystem && !ignoreOnwership)
                            {
                                backlog::post("(" + name + ")-associated-MI (" + name_mi + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                            }
                            else if (it->second->assetOwner != nullptr && !ignoreOnwership)
                            {
                                AssetVID vid_asset = GetAssetOwner(it->first);
                                utils::Entity ett_asset = utils::Entity::import(vid_asset);
                                std::string name_asset = ncm.GetName(ett_asset);
                                assert(vid_asset);
                                backlog::post("(" + name + ")-associated-MI (" + name_mi + ") is asset(" + name_asset + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                            }
                            else
                            {

                                isRenderableResource = true;
                                ncm.RemoveEntity(ett_mi); // explicitly 
                                em.destroy(ett_mi); // double check
                                it = miResMap_.erase(it); // call destructor...
                                backlog::post("(" + name + ")-associated-MI (" + name_mi + ") has been removed", backlog::LogLevel::Default);
                            }

                        }
                        else
                        {
                            ++it;
                        }
                    }
                    materialResMap_.erase(it_m); // call destructor...
                    backlog::post("Material (" + name + ") has been removed", backlog::LogLevel::Default);
                }
                // caution: 
                // before destroying the material,
                // destroy the associated material instances
            }
            auto it_mi = miResMap_.find(vid);
            if (it_mi != miResMap_.end())
            {
                VzMIRes& mi_res = *it_mi->second.get();
                if (mi_res.isSystem && !ignoreOnwership)
                {
                    backlog::post("MI (" + name + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else if (mi_res.assetOwner && !ignoreOnwership)
                {
                    AssetVID vid_asset = GetAssetOwner(it_mi->first);
                    utils::Entity ett_asset = utils::Entity::import(vid_asset);
                    std::string name_asset = ncm.GetName(ett_asset);
                    assert(vid_asset);
                    backlog::post("MI (" + name + ") is asset(" + name_asset + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else
                {
                    for (auto& it : textureResMap_)
                    {
                        VzTextureRes* tex_res = it.second.get();
                        tex_res->assignedMIs.erase(it_mi->first);
                    }

                    miResMap_.erase(it_mi); // call destructor...
                    isRenderableResource = true;
                    backlog::post("MI (" + name + ") has been removed", backlog::LogLevel::Default);
                }
            }
            auto it_geo = geometryResMap_.find(vid);
            if (it_geo != geometryResMap_.end())
            {
                VzGeometryRes& geo_res = *it_geo->second.get();
                if (geo_res.isSystem && !ignoreOnwership)
                {
                    backlog::post("Geometry (" + name + ") is system-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else if (geo_res.assetOwner && !ignoreOnwership)
                {
                    AssetVID vid_asset = GetAssetOwner(it_geo->first);
                    utils::Entity ett_asset = utils::Entity::import(vid_asset);
                    std::string name_asset = ncm.GetName(ett_asset);
                    assert(vid_asset);
                    backlog::post("Geometry (" + name + ") is asset(" + name_asset + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                    return false;
                }
                else
                {
                    geometryResMap_.erase(it_geo); // call destructor...
                    isRenderableResource = true;
                    backlog::post("Geometry (" + name + ") has been removed", backlog::LogLevel::Default);
                }
            }
            auto it_font = fontResMap_.find(vid);
            if (it_font != fontResMap_.end())
            {
                fontResMap_.erase(it_font); // call destructor...
                backlog::post("Font (" + name + ") has been removed", backlog::LogLevel::Default);
            }

            if (isRenderableResource)
            {
                for (auto& it_res : actorResMap_)
                {
                    VzActorRes& actor_res = *it_res.second.get();
                    if (geometryResMap_.find(actor_res.GetGeometryVid()) == geometryResMap_.end())
                    {
                        actor_res.SetGeometry(INVALID_VID);
                    }

                    std::vector<MInstanceVID> mis = actor_res.GetMIVids();
                    for (int i = 0, n = (int)mis.size(); i < n; ++i)
                    {
                        if (miResMap_.find(mis[i]) == miResMap_.end())
                        {
                            actor_res.SetMI(INVALID_VID, i);
                        }
                    }
                }
            }

            auto it_camres = camResMap_.find(vid);
            if (it_camres != camResMap_.end())
            {
                VzCameraRes& cam_res = *it_camres->second.get();
                Cube* cam_cube = cam_res.GetCameraCube();
                if (cam_cube)
                {
                    SceneVID vid_scene = GetSceneVidBelongTo(vid);
                    Scene* scene = GetScene(vid_scene);
                    if (scene)
                    {
                        scene->remove(cam_cube->getSolidRenderable());
                        scene->remove(cam_cube->getWireFrameRenderable());
                    }
                }
                camResMap_.erase(it_camres); // call destructor
            }

            auto it_light = lightResMap_.find(vid);
            if (it_light != lightResMap_.end())
            {
                lightResMap_.erase(it_light); // call destructor
            }

            if (int vid_assetowner = GetAssetOwner(vid))
            {
                utils::Entity ett_assetowner = utils::Entity::import(vid_assetowner);
                std::string name_assetowner = ncm.GetName(ett_assetowner);
                backlog::post("Component (" + name + ") is asset(" + name_assetowner + ")-owned component, thereby preserved.", backlog::LogLevel::Warning);
                return false;
            }

            VzAssetRes* asset_res = GetAssetRes(vid);
            if (asset_res)
            {
                vGltfIo.assetLoader->destroyAsset((gltfio::FFilamentAsset*)asset_res->asset);
                for (auto& it : asset_res->assetOwnershipComponents)
                {
                    RemoveComponent(it, true);
                }
                assetResMap_.erase(vid);
            }

#pragma endregion 
            // the remaining etts (not engine-destory group)

            vzCompMap_.erase(vid);

            actorSceneMap_.erase(vid);
            actorResMap_.erase(vid);
            lightSceneMap_.erase(vid);
            lightResMap_.erase(vid);
            camSceneMap_.erase(vid);
            camResMap_.erase(vid);
            renderPathMap_.erase(vid);

            for (auto& it : scenes_)
            {
                Scene* scene = it.second;
                scene->remove(ett);
            }
        }

        em.destroy(ett); // the associated engine components having the entity will be removed 
        gEngine->destroy(ett);

        return true;
    }

    void VzEngineApp::CancelAyncLoad()
    {
        if (vGltfIo.resourceLoader)
        {
            vGltfIo.resourceLoader->asyncCancelLoad();
        }
    }
    void VzEngineApp::Initialize()
    {
        ResourceConfiguration configuration = {};
        configuration.engine = gEngine;
        configuration.gltfPath = "";
        configuration.normalizeSkinningWeights = true;

        vGltfIo.resourceLoader = new gltfio::ResourceLoader(configuration);
        vGltfIo.stbDecoder = createStbProvider(gEngine);
        vGltfIo.ktxDecoder = createKtx2Provider(gEngine);
        vGltfIo.resourceLoader->addTextureProvider("image/png", vGltfIo.stbDecoder);
        vGltfIo.resourceLoader->addTextureProvider("image/jpeg", vGltfIo.stbDecoder);
        vGltfIo.resourceLoader->addTextureProvider("image/ktx2", vGltfIo.ktxDecoder);

        auto& ncm = VzNameCompManager::Get();
        vGltfIo.assetLoader = new gltfio::VzAssetLoader({ gEngine, gMaterialProvider, (NameComponentManager*)&ncm });
        vGltfIo.assetExpoter = new gltfio::VzAssetExpoter();

        FT_Init_FreeType(&ftLibrary);
    }
    void VzEngineApp::Destroy()
    {
        // dummy call //

        if (vGltfIo.resourceLoader)
        {
            vGltfIo.resourceLoader->asyncCancelLoad();

            for (auto it = textureResMap_.begin(); it != textureResMap_.end(); it++)
            {
                it->second->isAsyncLocked = false;
            }
        }

        //std::unordered_map<SceneVID, filament::Scene*> scenes_;
        //// note a VzRenderPath involves a view that includes
        //// 1. camera and 2. scene
        //std::unordered_map<CamVID, VzRenderPath> renderPathMap_;
        //std::unordered_map<ActorVID, SceneVID> actorSceneMap_;
        //std::unordered_map<LightVID, SceneVID> lightSceneMap_;
        //
        //// Resources
        //std::unordered_map<GeometryVID, filamesh::MeshReader::Mesh> geometryResMap_;
        //std::unordered_map<MaterialVID, filament::Material*> materialResMap_;
        //std::unordered_map<MaterialInstanceVID, filament::MaterialInstance*> miResMap_;

        // iteratively calling RemoveEntity of the following keys 
        destroyTarget(camSceneMap_);
        destroyTarget(actorSceneMap_); // including actorResMap_
        destroyTarget(lightSceneMap_); // including lightResMap_
        destroyTarget(renderPathMap_);
        destroyTarget(scenes_);
        destroyTarget(geometryResMap_);
        destroyTarget(textureResMap_);
        destroyTarget(miResMap_);
        destroyTarget(materialResMap_);
        destroyTarget(fontResMap_);

        if (assetResMap_.size() > 0) {
            for (auto& it : assetResMap_)
            {
                VzAssetRes& asset_res = *it.second.get();
                // For membership of each gltf component belongs to VZM
                filament::gltfio::FFilamentAsset* fasset = downcast(asset_res.asset);
                {
                    // Destroy gltfio node components.
                    for (auto& entity : fasset->mEntities) {
                        fasset->mNodeManager->destroy(entity);
                    }
                    // Destroy gltfio trs transform components.
                    for (auto& entity : fasset->mEntities) {
                        fasset->mTrsTransformManager->destroy(entity);
                    }
                }
                fasset->mEntities.clear(); // including... animation skeleton bones
                fasset->detachFilamentComponents();
                fasset->mVertexBuffers.clear();
                fasset->mIndexBuffers.clear();
                fasset->mTextures.clear();
                //fasset->mBufferObjects.clear();
                //fasset->mTextures.clear(); 
                fasset->mMorphTargetBuffers.clear();

                for (FFilamentInstance* instance : fasset->mInstances) {
                    instance->mMaterialInstances.clear(); // do not 
                    delete instance;
                }
                fasset->mInstances.clear();

                vGltfIo.assetLoader->destroyAsset(fasset); // involving skeletons...
            }
            assetResMap_.clear();
        }

        vGltfIo.Destory();

        FT_Done_FreeType(ftLibrary);
    }
#pragma endregion
}


#ifdef WIN32
#include <windows.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace vzm::backlog
{
    void setConsoleColor(unsigned short color) {
#ifdef WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
#endif
    }

    void post(const std::string& input, LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Default:
            setConsoleColor(10);
            std::cout << "[INFO] ";
            setConsoleColor(7);
            utils::slog.i << input + "\n";
            break;
        case LogLevel::Warning:
            setConsoleColor(14);
            std::cout << "[WARNING] ";
            setConsoleColor(7);
            utils::slog.w << input + "\n";
            break;
        case LogLevel::Error:
            setConsoleColor(12);
            std::cout << "[ERROR] ";
            setConsoleColor(7);
            utils::slog.e << input + "\n";
            break;
        default: return;
        }
        std::cout << input << std::endl;
    }
}
