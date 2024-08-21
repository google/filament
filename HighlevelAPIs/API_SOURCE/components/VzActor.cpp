#include "VzActor.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    void VzActor::SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        COMP_ACTOR(rcm, ett, ins, );
        rcm.setLayerMask(ins, layerBits, maskBits);
        UpdateTimeStamp();
    }
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
        rcm.setReceiveShadows(ins, false);
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
    void VzSpriteActor::EnableBillboard(const bool billboardEnabled)
    {
        // to do //
        UpdateTimeStamp();
    }
    void VzSpriteActor::SetGeometry(const float w, const float h, const float anchorU, const float anchorV)
    {
        VzActorRes* actor_res = gEngineApp.GetActorRes(GetVID());
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
            {{-half_size + offset_x,  half_size + offset_y, 0}, {0, 1}},
            {{ half_size + offset_x,  half_size + offset_y, 0}, {1, 1}}, 
            {{-half_size + offset_x, -half_size + offset_y, 0}, {0, 0}}, 
            {{ half_size + offset_x, -half_size + offset_y, 0}, {1, 0}} };
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

        utils::Entity ett_actor = utils::Entity::import(GetVID());
        builder
            .boundingBox(Box().set(aabb.min, aabb.max))
            .culling(actor_res->culling) // false
            .castShadows(actor_res->castShadow) // false
            .receiveShadows(actor_res->receiveShadow) // false
            .build(*gEngine, ett_actor);

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
