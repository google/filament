/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file  BlenderModifier.cpp
 *  @brief Implementation of some blender modifiers (i.e subdivision, mirror).
 */

#ifndef ASSIMP_BUILD_NO_BLEND_IMPORTER

#include "BlenderModifier.h"
#include <assimp/SceneCombiner.h>
#include "Subdivision.h"
#include <assimp/scene.h>
#include <memory>

#include <functional>

using namespace Assimp;
using namespace Assimp::Blender;

template <typename T> BlenderModifier* god() {
    return new T();
}

// add all available modifiers here
typedef BlenderModifier* (*fpCreateModifier)();
static const fpCreateModifier creators[] = {
        &god<BlenderModifier_Mirror>,
        &god<BlenderModifier_Subdivision>,

        NULL // sentinel
};

// ------------------------------------------------------------------------------------------------
// just testing out some new macros to simplify logging
#define ASSIMP_LOG_WARN_F(string,...)\
    DefaultLogger::get()->warn((Formatter::format(string),__VA_ARGS__))

#define ASSIMP_LOG_ERROR_F(string,...)\
    DefaultLogger::get()->error((Formatter::format(string),__VA_ARGS__))

#define ASSIMP_LOG_DEBUG_F(string,...)\
    DefaultLogger::get()->debug((Formatter::format(string),__VA_ARGS__))

#define ASSIMP_LOG_INFO_F(string,...)\
    DefaultLogger::get()->info((Formatter::format(string),__VA_ARGS__))


#define ASSIMP_LOG_WARN(string)\
    DefaultLogger::get()->warn(string)

#define ASSIMP_LOG_ERROR(string)\
    DefaultLogger::get()->error(string)

#define ASSIMP_LOG_DEBUG(string)\
    DefaultLogger::get()->debug(string)

#define ASSIMP_LOG_INFO(string)\
    DefaultLogger::get()->info(string)


// ------------------------------------------------------------------------------------------------
struct SharedModifierData : ElemBase
{
    ModifierData modifier;
};

// ------------------------------------------------------------------------------------------------
void BlenderModifierShowcase::ApplyModifiers(aiNode& out, ConversionData& conv_data, const Scene& in, const Object& orig_object )
{
    size_t cnt = 0u, ful = 0u;

    // NOTE: this cast is potentially unsafe by design, so we need to perform type checks before
    // we're allowed to dereference the pointers without risking to crash. We might still be
    // invoking UB btw - we're assuming that the ModifierData member of the respective modifier
    // structures is at offset sizeof(vftable) with no padding.
    const SharedModifierData* cur = static_cast<const SharedModifierData *> ( orig_object.modifiers.first.get() );
    for (; cur; cur =  static_cast<const SharedModifierData *> ( cur->modifier.next.get() ), ++ful) {
        ai_assert(cur->dna_type);

        const Structure* s = conv_data.db.dna.Get( cur->dna_type );
        if (!s) {
            ASSIMP_LOG_WARN_F("BlendModifier: could not resolve DNA name: ",cur->dna_type);
            continue;
        }

        // this is a common trait of all XXXMirrorData structures in BlenderDNA
        const Field* f = s->Get("modifier");
        if (!f || f->offset != 0) {
            ASSIMP_LOG_WARN("BlendModifier: expected a `modifier` member at offset 0");
            continue;
        }

        s = conv_data.db.dna.Get( f->type );
        if (!s || s->name != "ModifierData") {
            ASSIMP_LOG_WARN("BlendModifier: expected a ModifierData structure as first member");
            continue;
        }

        // now, we can be sure that we should be fine to dereference *cur* as
        // ModifierData (with the above note).
        const ModifierData& dat = cur->modifier;

        const fpCreateModifier* curgod = creators;
        std::vector< BlenderModifier* >::iterator curmod = cached_modifiers->begin(), endmod = cached_modifiers->end();

        for (;*curgod;++curgod,++curmod) { // allocate modifiers on the fly
            if (curmod == endmod) {
                cached_modifiers->push_back((*curgod)());

                endmod = cached_modifiers->end();
                curmod = endmod-1;
            }

            BlenderModifier* const modifier = *curmod;
            if(modifier->IsActive(dat)) {
                modifier->DoIt(out,conv_data,*static_cast<const ElemBase *>(cur),in,orig_object);
                cnt++;

                curgod = NULL;
                break;
            }
        }
        if (curgod) {
            ASSIMP_LOG_WARN_F("Couldn't find a handler for modifier: ",dat.name);
        }
    }

    // Even though we managed to resolve some or all of the modifiers on this
    // object, we still can't say whether our modifier implementations were
    // able to fully do their job.
    if (ful) {
        ASSIMP_LOG_DEBUG_F("BlendModifier: found handlers for ",cnt," of ",ful," modifiers on `",orig_object.id.name,
            "`, check log messages above for errors");
    }
}



// ------------------------------------------------------------------------------------------------
bool BlenderModifier_Mirror :: IsActive (const ModifierData& modin)
{
    return modin.type == ModifierData::eModifierType_Mirror;
}

// ------------------------------------------------------------------------------------------------
void  BlenderModifier_Mirror :: DoIt(aiNode& out, ConversionData& conv_data,  const ElemBase& orig_modifier,
    const Scene& /*in*/,
    const Object& orig_object )
{
    // hijacking the ABI, see the big note in BlenderModifierShowcase::ApplyModifiers()
    const MirrorModifierData& mir = static_cast<const MirrorModifierData&>(orig_modifier);
    ai_assert(mir.modifier.type == ModifierData::eModifierType_Mirror);

    conv_data.meshes->reserve(conv_data.meshes->size() + out.mNumMeshes);

    // XXX not entirely correct, mirroring on two axes results in 4 distinct objects in blender ...

    // take all input meshes and clone them
    for (unsigned int i = 0; i < out.mNumMeshes; ++i) {
        aiMesh* mesh;
        SceneCombiner::Copy(&mesh,conv_data.meshes[out.mMeshes[i]]);

        const float xs = mir.flag & MirrorModifierData::Flags_AXIS_X ? -1.f : 1.f;
        const float ys = mir.flag & MirrorModifierData::Flags_AXIS_Y ? -1.f : 1.f;
        const float zs = mir.flag & MirrorModifierData::Flags_AXIS_Z ? -1.f : 1.f;

        if (mir.mirror_ob) {
            const aiVector3D center( mir.mirror_ob->obmat[3][0],mir.mirror_ob->obmat[3][1],mir.mirror_ob->obmat[3][2] );
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                aiVector3D& v = mesh->mVertices[i];

                v.x = center.x + xs*(center.x - v.x);
                v.y = center.y + ys*(center.y - v.y);
                v.z = center.z + zs*(center.z - v.z);
            }
        }
        else {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                aiVector3D& v = mesh->mVertices[i];
                v.x *= xs;v.y *= ys;v.z *= zs;
            }
        }

        if (mesh->mNormals) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                aiVector3D& v = mesh->mNormals[i];
                v.x *= xs;v.y *= ys;v.z *= zs;
            }
        }

        if (mesh->mTangents) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                aiVector3D& v = mesh->mTangents[i];
                v.x *= xs;v.y *= ys;v.z *= zs;
            }
        }

        if (mesh->mBitangents) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                aiVector3D& v = mesh->mBitangents[i];
                v.x *= xs;v.y *= ys;v.z *= zs;
            }
        }

        const float us = mir.flag & MirrorModifierData::Flags_MIRROR_U ? -1.f : 1.f;
        const float vs = mir.flag & MirrorModifierData::Flags_MIRROR_V ? -1.f : 1.f;

        for (unsigned int n = 0; mesh->HasTextureCoords(n); ++n) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                aiVector3D& v = mesh->mTextureCoords[n][i];
                v.x *= us;v.y *= vs;
            }
        }

        // Only reverse the winding order if an odd number of axes were mirrored.
        if (xs * ys * zs < 0) {
            for( unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace& face = mesh->mFaces[i];
                for( unsigned int fi = 0; fi < face.mNumIndices / 2; ++fi)
                    std::swap( face.mIndices[fi], face.mIndices[face.mNumIndices - 1 - fi]);
            }
        }

        conv_data.meshes->push_back(mesh);
    }
    unsigned int* nind = new unsigned int[out.mNumMeshes*2];

    std::copy(out.mMeshes,out.mMeshes+out.mNumMeshes,nind);
    std::transform(out.mMeshes,out.mMeshes+out.mNumMeshes,nind+out.mNumMeshes,
        [&out](unsigned int n) { return out.mNumMeshes + n; });

    delete[] out.mMeshes;
    out.mMeshes = nind;
    out.mNumMeshes *= 2;

    ASSIMP_LOG_INFO_F("BlendModifier: Applied the `Mirror` modifier to `",
        orig_object.id.name,"`");
}

// ------------------------------------------------------------------------------------------------
bool BlenderModifier_Subdivision :: IsActive (const ModifierData& modin)
{
    return modin.type == ModifierData::eModifierType_Subsurf;
}

// ------------------------------------------------------------------------------------------------
void  BlenderModifier_Subdivision :: DoIt(aiNode& out, ConversionData& conv_data,  const ElemBase& orig_modifier,
    const Scene& /*in*/,
    const Object& orig_object )
{
    // hijacking the ABI, see the big note in BlenderModifierShowcase::ApplyModifiers()
    const SubsurfModifierData& mir = static_cast<const SubsurfModifierData&>(orig_modifier);
    ai_assert(mir.modifier.type == ModifierData::eModifierType_Subsurf);

    Subdivider::Algorithm algo;
    switch (mir.subdivType)
    {
    case SubsurfModifierData::TYPE_CatmullClarke:
        algo = Subdivider::CATMULL_CLARKE;
        break;

    case SubsurfModifierData::TYPE_Simple:
        ASSIMP_LOG_WARN("BlendModifier: The `SIMPLE` subdivision algorithm is not currently implemented, using Catmull-Clarke");
        algo = Subdivider::CATMULL_CLARKE;
        break;

    default:
        ASSIMP_LOG_WARN_F("BlendModifier: Unrecognized subdivision algorithm: ",mir.subdivType);
        return;
    };

    std::unique_ptr<Subdivider> subd(Subdivider::Create(algo));
    ai_assert(subd);
    if ( conv_data.meshes->empty() ) {
        return;
    }
    aiMesh** const meshes = &conv_data.meshes[conv_data.meshes->size() - out.mNumMeshes];
    std::unique_ptr<aiMesh*[]> tempmeshes(new aiMesh*[out.mNumMeshes]());

    subd->Subdivide(meshes,out.mNumMeshes,tempmeshes.get(),std::max( mir.renderLevels, mir.levels ),true);
    std::copy(tempmeshes.get(),tempmeshes.get()+out.mNumMeshes,meshes);

    ASSIMP_LOG_INFO_F("BlendModifier: Applied the `Subdivision` modifier to `",
        orig_object.id.name,"`");
}

#endif // ASSIMP_BUILD_NO_BLEND_IMPORTER
