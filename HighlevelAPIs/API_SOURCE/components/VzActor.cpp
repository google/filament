#include "VzActor.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    void VzBaseActor::SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setLayerMask(ins, layerBits, maskBits);
        UpdateTimeStamp();
    }
}

namespace vzm
{
    void VzActor::SetMI(const VID vidMI, const int slot)
    {
        VzMIRes* mi_res = gEngineApp.GetMIRes(vidMI);
        if (mi_res == nullptr)
        {
            backlog::post("invalid material instance!", backlog::LogLevel::Error);
            return;
        }
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        if (!actor_res->SetMI(vidMI, slot))
        {
            return;
        }
        auto& rcm = gEngine->getRenderableManager();
        utils::Entity ett_actor = utils::Entity::import(GetVID());
        auto ins = rcm.getInstance(ett_actor);
        rcm.setMaterialInstanceAt(ins, slot, mi_res->mi);
        UpdateTimeStamp();
    }
    void VzActor::SetCastShadows(const bool enabled)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setCastShadows(ins, enabled);
    }
    void VzActor::SetReceiveShadows(const bool enabled)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setReceiveShadows(ins, enabled);
    }
    void VzActor::SetScreenSpaceContactShadows(const bool enabled)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setScreenSpaceContactShadows(ins, enabled);
    }
    void VzActor::SetRenderableRes(const VID vidGeo, const std::vector<VID>& vidMIs)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        actor_res->SetGeometry(vidGeo);
        actor_res->SetMIs(vidMIs);
        gEngineApp.BuildRenderable(GetVID());
        UpdateTimeStamp();
    }
    std::vector<VID> VzActor::GetMIs()
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        return actor_res->GetMIVids();
    }
    VID VzActor::GetMI(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        return actor_res->GetMIVid(slot);
    }
    VID VzActor::GetMaterial(const int slot)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        MInstanceVID vid_mi = actor_res->GetMIVid(slot);
        VzMIRes* mi_res = gEngineApp.GetMIRes(vid_mi);
        if (mi_res == nullptr)
        {
            return INVALID_VID;
        }
        MaterialInstance* mi = mi_res->mi;
        assert(mi);
        const Material* mat = mi->getMaterial();
        assert(mat != nullptr);
        return gEngineApp.FindMaterialVID(mat);
    }
    VID VzActor::GetGeometry()
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        return actor_res->GetGeometryVid();
    }

}


namespace vzm
{
    void VzBaseSprite::EnableBillboard(const bool billboardEnabled)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(baseActor_->GetVID());
        actor_res->isBillboard = billboardEnabled;
        baseActor_->UpdateTimeStamp();
    }

    void VzBaseSprite::SetRotatation(const float rotDeg)
    {
        // TO DO
        baseActor_->UpdateTimeStamp();
    }
}

namespace vzm
{
    void buildQuadGeometry(const VID vid, const float w, const float h, const float anchorU, const float anchorV)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(vid);
        assert(actor_res->isSprite);
        if (actor_res->intrinsicVB) gEngine->destroy(actor_res->intrinsicVB);
        if (actor_res->intrinsicIB) gEngine->destroy(actor_res->intrinsicIB);

        struct SpriteVertex {
            float3 position;
            float2 uv;
        };
        float half_size = w * 0.5f;
        float offset_x = (0.5f - anchorU) * half_size;
        float offset_y = (0.5f - anchorV) * half_size;
        SpriteVertex kQuadVertices[4] = {
            {{-half_size + offset_x,  half_size + offset_y, 0}, {0, 0}},
            {{ half_size + offset_x,  half_size + offset_y, 0}, {1, 0}},
            {{-half_size + offset_x, -half_size + offset_y, 0}, {0, 1}},
            {{ half_size + offset_x, -half_size + offset_y, 0}, {1, 1}} };
        uint16_t kQuadIndices[6] = { 0, 2, 1, 1, 2, 3 };

        memcpy(&actor_res->intrinsicCache[0], kQuadVertices, 80);
        memcpy(&actor_res->intrinsicCache[80], kQuadIndices, 12);

        actor_res->intrinsicVB = VertexBuffer::Builder()
            .vertexCount(4)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, sizeof(SpriteVertex))
            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, sizeof(float3), sizeof(SpriteVertex))
            .build(*gEngine);
        actor_res->intrinsicVB->setBufferAt(*gEngine, 0,
            VertexBuffer::BufferDescriptor(&actor_res->intrinsicCache[0], 80, nullptr));

        // Create quad index buffer.
        actor_res->intrinsicIB = IndexBuffer::Builder()
            .indexCount(6)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*gEngine);
        actor_res->intrinsicIB->setBuffer(*gEngine, IndexBuffer::BufferDescriptor(&actor_res->intrinsicCache[80], 12, nullptr));
        Aabb aabb;
        aabb.min = { -half_size + offset_x, -half_size + offset_y, -0.5 };
        aabb.max = { half_size + offset_x, half_size + offset_y, 0.5 };

        RenderableManager::Builder builder(1);

        MaterialInstance* mi = gEngineApp.GetMIRes(actor_res->GetMIVids()[0])->mi;
        assert(mi);
        builder.material(0, mi);
        builder.geometry(0, RenderableManager::PrimitiveType::TRIANGLES, actor_res->intrinsicVB, actor_res->intrinsicIB);

        utils::Entity ett_actor = utils::Entity::import(vid);
        builder
            .boundingBox(Box().set(aabb.min, aabb.max))
            .culling(actor_res->culling) // false
            .castShadows(actor_res->castShadow) // false
            .receiveShadows(actor_res->receiveShadow) // false
            .build(*gEngine, ett_actor);
    }

    void VzSpriteActor::SetGeometry(const float w, const float h, const float anchorU, const float anchorV)
    {
        buildQuadGeometry(GetVID(), w, h, anchorU, anchorV);
        UpdateTimeStamp();
    }

    void VzSpriteActor::SetTexture(const VID vidTexture)
    {
        VzTextureRes* tex_res = gEngineApp.GetTextureRes(vidTexture);
        if (tex_res->texture == nullptr) {
            backlog::post("invalid texture!", backlog::LogLevel::Error);
            return;
        }

        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        MaterialInstance* mi = gEngineApp.GetMIRes(actor_res->GetMIVid(0))->mi;
        assert(mi);

        mi->setParameter("baseColorMap", tex_res->texture, tex_res->sampler);
        //mi->getMaterial()->get
        tex_res->assignedMIs.insert(GetVID());

        UpdateTimeStamp();
    }
}


#include "../../libs/imageio/include/imageio/ImageDecoder.h"
#include <fstream>
#include <iostream>

namespace vzm
{
    void VzTextSpriteActor::SetText(const std::string& text, const float fontHeight, const float anchorU, const float anchorV)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
        assert(actor_res->isSprite);
        if (actor_res->intrinsicTexture) gEngine->destroy(actor_res->intrinsicTexture);

        size_t text_image_w = 0, text_image_h = 0;
        unsigned char* text_image = nullptr;
        {
            // TO DO //
            // generate a text image (rgba, backgournd alpha is zero) from "text"
            // this is supposed to update the following
            //      * image width and height in pixels : text_image_w, text_image_h
            //      * unsigned char* as image (rgba) buffer pointer (the allocation will be owned by VzTextSpriteActor)
            using namespace image;
            std::string test_file_name = "../assets/textimage.png";
            std::ifstream inputStream(test_file_name, std::ios::binary);
            image::LinearImage* image = new LinearImage(ImageDecoder::decode(
                inputStream, test_file_name, ImageDecoder::ColorSpace::SRGB));
            if (!image->isValid()) {
                backlog::post("The input image is invalid:: " + test_file_name, backlog::LogLevel::Error);
                return;
            }
            inputStream.close();

            // channels must be 4 (due to alpha channel)
            uint32_t channels = image->getChannels();
            text_image_w = image->getWidth();
            text_image_h = image->getHeight();
            Texture* texture = Texture::Builder()
                .width(text_image_w)
                .height(text_image_h)
                .levels(0xff)
                .format(channels == 3 ?
                    Texture::InternalFormat::RGB16F : Texture::InternalFormat::RGBA16F)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .build(*gEngine);

            Texture::PixelBufferDescriptor::Callback freeCallback = [](void* buf, size_t, void* data) {
                delete reinterpret_cast<LinearImage*>(data);
                };

            Texture::PixelBufferDescriptor buffer(
                image->getPixelRef(),
                size_t(text_image_w * text_image_h * channels * sizeof(float)),
                channels == 3 ? Texture::Format::RGB : Texture::Format::RGBA,
                Texture::Type::FLOAT,
                freeCallback,
                image
            );

            texture->setImage(*gEngine, 0, std::move(buffer));
            texture->generateMipmaps(*gEngine);

            actor_res->intrinsicTexture = texture;
        }

        MaterialInstance* mi = gEngineApp.GetMIRes(actor_res->GetMIVids()[0])->mi;
        TextureSampler sampler;
        sampler.setMagFilter(TextureSampler::MagFilter::LINEAR);
        sampler.setMinFilter(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR);
        sampler.setWrapModeS(TextureSampler::WrapMode::REPEAT);
        sampler.setWrapModeT(TextureSampler::WrapMode::REPEAT);
        mi->setParameter("baseColorMap", actor_res->intrinsicTexture, sampler);

        const float w = fontHeight / (float)text_image_h * text_image_w;
        const float h = fontHeight;
        //if (actor_res->intrinsicVB) gEngine->destroy(actor_res->intrinsicVB);
        //if (actor_res->intrinsicIB) gEngine->destroy(actor_res->intrinsicIB);
        buildQuadGeometry(GetVID(), w, h, anchorU, anchorV);

        UpdateTimeStamp();
    }
}
