/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team



All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

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
---------------------------------------------------------------------------
*/

/** @file  MS3DLoader.cpp
 *  @brief Implementation of the Ms3D importer class.
 *  Written against http://chumbalum.swissquake.ch/ms3d/ms3dspec.txt
 */


#ifndef ASSIMP_BUILD_NO_MS3D_IMPORTER

// internal headers
#include "MS3DLoader.h"
#include <assimp/StreamReader.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/IOSystem.hpp>
#include <assimp/importerdesc.h>
#include <map>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Milkshape 3D Importer",
    "",
    "",
    "http://chumbalum.swissquake.ch/",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "ms3d"
};

// ASSIMP_BUILD_MS3D_ONE_NODE_PER_MESH
//   (enable old code path, which generates extra nodes per mesh while
//    the newer code uses aiMesh::mName to express the name of the
//    meshes (a.k.a. groups in MS3D))

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
MS3DImporter::MS3DImporter()
    : mScene()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
MS3DImporter::~MS3DImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool MS3DImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    // first call - simple extension check
    const std::string extension = GetExtension(pFile);
    if (extension == "ms3d") {
        return true;
    }

    // second call - check for magic identifiers
    else if (!extension.length() || checkSig)   {
        if (!pIOHandler) {
            return true;
        }
        const char* tokens[] = {"MS3D000000"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* MS3DImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
void ReadColor(StreamReaderLE& stream, aiColor4D& ambient)
{
    // aiColor4D is packed on gcc, implicit binding to float& fails therefore.
    stream >> (float&)ambient.r >> (float&)ambient.g >> (float&)ambient.b >> (float&)ambient.a;
}

// ------------------------------------------------------------------------------------------------
void ReadVector(StreamReaderLE& stream, aiVector3D& pos)
{
    // See note in ReadColor()
    stream >> (float&)pos.x >> (float&)pos.y >> (float&)pos.z;
}

// ------------------------------------------------------------------------------------------------
template<typename T>
void MS3DImporter :: ReadComments(StreamReaderLE& stream, std::vector<T>& outp)
{
    uint16_t cnt;
    stream >> cnt;

    for(unsigned int i = 0; i < cnt; ++i) {
        uint32_t index, clength;
        stream >> index >> clength;

        if(index >= outp.size()) {
            ASSIMP_LOG_WARN("MS3D: Invalid index in comment section");
        }
        else if (clength > stream.GetRemainingSize()) {
            throw DeadlyImportError("MS3D: Failure reading comment, length field is out of range");
        }
        else {
            outp[index].comment = std::string(reinterpret_cast<char*>(stream.GetPtr()),clength);
        }
        stream.IncPtr(clength);
    }
}

// ------------------------------------------------------------------------------------------------
template <typename T, typename T2, typename T3> bool inrange(const T& in, const T2& lower, const T3& higher)
{
    return in > lower && in <= higher;
}

// ------------------------------------------------------------------------------------------------
void MS3DImporter :: CollectChildJoints(const std::vector<TempJoint>& joints,
    std::vector<bool>& hadit,
    aiNode* nd,
    const aiMatrix4x4& absTrafo)
{
    unsigned int cnt = 0;
    for(size_t i = 0; i < joints.size(); ++i) {
        if (!hadit[i] && !strcmp(joints[i].parentName,nd->mName.data)) {
            ++cnt;
        }
    }

    nd->mChildren = new aiNode*[nd->mNumChildren = cnt];
    cnt = 0;
    for(size_t i = 0; i < joints.size(); ++i) {
        if (!hadit[i] && !strcmp(joints[i].parentName,nd->mName.data)) {
            aiNode* ch = nd->mChildren[cnt++] = new aiNode(joints[i].name);
            ch->mParent = nd;

            ch->mTransformation = aiMatrix4x4::Translation(joints[i].position,aiMatrix4x4()=aiMatrix4x4())*
                aiMatrix4x4().FromEulerAnglesXYZ(joints[i].rotation);

            const aiMatrix4x4 abs = absTrafo*ch->mTransformation;
            for(unsigned int a = 0; a < mScene->mNumMeshes; ++a) {
                aiMesh* const msh = mScene->mMeshes[a];
                for(unsigned int n = 0; n < msh->mNumBones; ++n) {
                    aiBone* const bone = msh->mBones[n];

                    if(bone->mName == ch->mName) {
                        bone->mOffsetMatrix = aiMatrix4x4(abs).Inverse();
                    }
                }
            }

            hadit[i] = true;
            CollectChildJoints(joints,hadit,ch,abs);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void MS3DImporter :: CollectChildJoints(const std::vector<TempJoint>& joints, aiNode* nd)
{
     std::vector<bool> hadit(joints.size(),false);
     aiMatrix4x4 trafo;

     CollectChildJoints(joints,hadit,nd,trafo);
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void MS3DImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    StreamReaderLE stream(pIOHandler->Open(pFile,"rb"));

    // CanRead() should have done this already
    char head[10];
    int32_t version;

    mScene = pScene;


    // 1 ------------ read into temporary data structures mirroring the original file

    stream.CopyAndAdvance(head,10);
    stream >> version;
    if (strncmp(head,"MS3D000000",10)) {
        throw DeadlyImportError("Not a MS3D file, magic string MS3D000000 not found: "+pFile);
    }

    if (version != 4) {
        throw DeadlyImportError("MS3D: Unsupported file format version, 4 was expected");
    }

    uint16_t verts;
    stream >> verts;

    std::vector<TempVertex> vertices(verts);
    for (unsigned int i = 0; i < verts; ++i) {
        TempVertex& v = vertices[i];

        stream.IncPtr(1);
        ReadVector(stream,v.pos);
        v.bone_id[0] = stream.GetI1();
        v.ref_cnt = stream.GetI1();

        v.bone_id[1] = v.bone_id[2] = v.bone_id[3] = UINT_MAX;
        v.weights[1] = v.weights[2] = v.weights[3] = 0.f;
        v.weights[0] = 1.f;
    }

    uint16_t tris;
    stream >> tris;

    std::vector<TempTriangle> triangles(tris);
    for (unsigned int i = 0;i < tris; ++i) {
        TempTriangle& t = triangles[i];

        stream.IncPtr(2);
        for (unsigned int i = 0; i < 3; ++i) {
            t.indices[i] = stream.GetI2();
        }

        for (unsigned int i = 0; i < 3; ++i) {
            ReadVector(stream,t.normals[i]);
        }

        for (unsigned int i = 0; i < 3; ++i) {
            stream >> (float&)(t.uv[i].x); // see note in ReadColor()
        }
        for (unsigned int i = 0; i < 3; ++i) {
            stream >> (float&)(t.uv[i].y);
        }

        t.sg    = stream.GetI1();
        t.group = stream.GetI1();
    }

    uint16_t grp;
    stream >> grp;

    bool need_default = false;
    std::vector<TempGroup> groups(grp);
    for (unsigned int i = 0;i < grp; ++i) {
        TempGroup& t = groups[i];

        stream.IncPtr(1);
        stream.CopyAndAdvance(t.name,32);

        t.name[32] = '\0';
        uint16_t num;
        stream >> num;

        t.triangles.resize(num);
        for (unsigned int i = 0; i < num; ++i) {
            t.triangles[i] = stream.GetI2();
        }
        t.mat = stream.GetI1();
        if (t.mat == UINT_MAX) {
            need_default = true;
        }
    }

    uint16_t mat;
    stream >> mat;

    std::vector<TempMaterial> materials(mat);
    for (unsigned int i = 0;i < mat; ++i) {
        TempMaterial& t = materials[i];

        stream.CopyAndAdvance(t.name,32);
        t.name[32] = '\0';

        ReadColor(stream,t.ambient);
        ReadColor(stream,t.diffuse);
        ReadColor(stream,t.specular);
        ReadColor(stream,t.emissive);
        stream >> t.shininess  >> t.transparency;

        stream.IncPtr(1);

        stream.CopyAndAdvance(t.texture,128);
        t.texture[128] = '\0';

        stream.CopyAndAdvance(t.alphamap,128);
        t.alphamap[128] = '\0';
    }

    float animfps, currenttime;
    uint32_t totalframes;
    stream >> animfps >> currenttime >> totalframes;

    uint16_t joint;
    stream >> joint;

    std::vector<TempJoint> joints(joint);
    for(unsigned int i = 0; i < joint; ++i) {
        TempJoint& j = joints[i];

        stream.IncPtr(1);
        stream.CopyAndAdvance(j.name,32);
        j.name[32] = '\0';

        stream.CopyAndAdvance(j.parentName,32);
        j.parentName[32] = '\0';

        ReadVector(stream,j.rotation);
        ReadVector(stream,j.position);

        j.rotFrames.resize(stream.GetI2());
        j.posFrames.resize(stream.GetI2());

        for(unsigned int a = 0; a < j.rotFrames.size(); ++a) {
            TempKeyFrame& kf = j.rotFrames[a];
            stream >> kf.time;
            ReadVector(stream,kf.value);
        }
        for(unsigned int a = 0; a < j.posFrames.size(); ++a) {
            TempKeyFrame& kf = j.posFrames[a];
            stream >> kf.time;
            ReadVector(stream,kf.value);
        }
    }

    if(stream.GetRemainingSize() > 4) {
        uint32_t subversion;
        stream >> subversion;
        if (subversion == 1) {
            ReadComments<TempGroup>(stream,groups);
            ReadComments<TempMaterial>(stream,materials);
            ReadComments<TempJoint>(stream,joints);

            // model comment - print it for we have such a nice log.
            if (stream.GetI4()) {
                const size_t len = static_cast<size_t>(stream.GetI4());
                if (len > stream.GetRemainingSize()) {
                    throw DeadlyImportError("MS3D: Model comment is too long");
                }

                const std::string& s = std::string(reinterpret_cast<char*>(stream.GetPtr()),len);
                ASSIMP_LOG_DEBUG_F("MS3D: Model comment: ", s);
            }

            if(stream.GetRemainingSize() > 4 && inrange((stream >> subversion,subversion),1u,3u)) {
                for(unsigned int i = 0; i < verts; ++i) {
                    TempVertex& v = vertices[i];
                    v.weights[3]=1.f;
                    for(unsigned int n = 0; n < 3; v.weights[3]-=v.weights[n++]) {
                        v.bone_id[n+1] = stream.GetI1();
                        v.weights[n] = static_cast<float>(static_cast<unsigned int>(stream.GetI1()))/255.f;
                    }
                    stream.IncPtr((subversion-1)<<2u);
                }

                // even further extra data is not of interest for us, at least now now.
            }
        }
    }

    // 2 ------------ convert to proper aiXX data structures -----------------------------------

    if (need_default && materials.size()) {
        ASSIMP_LOG_WARN("MS3D: Found group with no material assigned, spawning default material");
        // if one of the groups has no material assigned, but there are other
        // groups with materials, a default material needs to be added (
        // scenepreprocessor adds a default material only if nummat==0).
        materials.push_back(TempMaterial());
        TempMaterial& m = materials.back();

        strcpy(m.name,"<MS3D_DefaultMat>");
        m.diffuse = aiColor4D(0.6f,0.6f,0.6f,1.0);
        m.transparency = 1.f;
        m.shininess = 0.f;

        // this is because these TempXXX struct's have no c'tors.
        m.texture[0] = m.alphamap[0] = '\0';

        for (unsigned int i = 0; i < groups.size(); ++i) {
            TempGroup& g = groups[i];
            if (g.mat == UINT_MAX) {
                g.mat = static_cast<unsigned int>(materials.size()-1);
            }
        }
    }

    // convert materials to our generic key-value dict-alike
    if (materials.size()) {
        pScene->mMaterials = new aiMaterial*[materials.size()];
        for (size_t i = 0; i < materials.size(); ++i) {

            aiMaterial* mo = new aiMaterial();
            pScene->mMaterials[pScene->mNumMaterials++] = mo;

            const TempMaterial& mi = materials[i];

            aiString tmp;
            if (0[mi.alphamap]) {
                tmp = aiString(mi.alphamap);
                mo->AddProperty(&tmp,AI_MATKEY_TEXTURE_OPACITY(0));
            }
            if (0[mi.texture]) {
                tmp = aiString(mi.texture);
                mo->AddProperty(&tmp,AI_MATKEY_TEXTURE_DIFFUSE(0));
            }
            if (0[mi.name]) {
                tmp = aiString(mi.name);
                mo->AddProperty(&tmp,AI_MATKEY_NAME);
            }

            mo->AddProperty(&mi.ambient,1,AI_MATKEY_COLOR_AMBIENT);
            mo->AddProperty(&mi.diffuse,1,AI_MATKEY_COLOR_DIFFUSE);
            mo->AddProperty(&mi.specular,1,AI_MATKEY_COLOR_SPECULAR);
            mo->AddProperty(&mi.emissive,1,AI_MATKEY_COLOR_EMISSIVE);

            mo->AddProperty(&mi.shininess,1,AI_MATKEY_SHININESS);
            mo->AddProperty(&mi.transparency,1,AI_MATKEY_OPACITY);

            const int sm = mi.shininess>0.f?aiShadingMode_Phong:aiShadingMode_Gouraud;
            mo->AddProperty(&sm,1,AI_MATKEY_SHADING_MODEL);
        }
    }

    // convert groups to meshes
    if (groups.empty()) {
        throw DeadlyImportError("MS3D: Didn't get any group records, file is malformed");
    }

    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes=static_cast<unsigned int>(groups.size())]();
    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {

        aiMesh* m = pScene->mMeshes[i] = new aiMesh();
        const TempGroup& g = groups[i];

        if (pScene->mNumMaterials && g.mat > pScene->mNumMaterials) {
            throw DeadlyImportError("MS3D: Encountered invalid material index, file is malformed");
        } // no error if no materials at all - scenepreprocessor adds one then

        m->mMaterialIndex  = g.mat;
        m->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

        m->mFaces = new aiFace[m->mNumFaces = static_cast<unsigned int>(g.triangles.size())];
        m->mNumVertices = m->mNumFaces*3;

        // storage for vertices - verbose format, as requested by the postprocessing pipeline
        m->mVertices = new aiVector3D[m->mNumVertices];
        m->mNormals  = new aiVector3D[m->mNumVertices];
        m->mTextureCoords[0] = new aiVector3D[m->mNumVertices];
        m->mNumUVComponents[0] = 2;

        typedef std::map<unsigned int,unsigned int> BoneSet;
        BoneSet mybones;

        for (unsigned int i = 0,n = 0; i < m->mNumFaces; ++i) {
            aiFace& f = m->mFaces[i];
            if (g.triangles[i]>triangles.size()) {
                throw DeadlyImportError("MS3D: Encountered invalid triangle index, file is malformed");
            }

            TempTriangle& t = triangles[g.triangles[i]];
            f.mIndices = new unsigned int[f.mNumIndices=3];

            for (unsigned int i = 0; i < 3; ++i,++n) {
                if (t.indices[i]>vertices.size()) {
                    throw DeadlyImportError("MS3D: Encountered invalid vertex index, file is malformed");
                }

                const TempVertex& v = vertices[t.indices[i]];
                for(unsigned int a = 0; a < 4; ++a) {
                    if (v.bone_id[a] != UINT_MAX) {
                        if (v.bone_id[a] >= joints.size()) {
                            throw DeadlyImportError("MS3D: Encountered invalid bone index, file is malformed");
                        }
                        if (mybones.find(v.bone_id[a]) == mybones.end()) {
                             mybones[v.bone_id[a]] = 1;
                        }
                        else ++mybones[v.bone_id[a]];
                    }
                }

                // collect vertex components
                m->mVertices[n] = v.pos;

                m->mNormals[n] = t.normals[i];
                m->mTextureCoords[0][n] = aiVector3D(t.uv[i].x,1.f-t.uv[i].y,0.0);
                f.mIndices[i] = n;
            }
        }

        // allocate storage for bones
        if(!mybones.empty()) {
            std::vector<unsigned int> bmap(joints.size());
            m->mBones = new aiBone*[mybones.size()]();
            for(BoneSet::const_iterator it = mybones.begin(); it != mybones.end(); ++it) {
                aiBone* const bn = m->mBones[m->mNumBones] = new aiBone();
                const TempJoint& jnt = joints[(*it).first];

                bn->mName.Set(jnt.name);
                bn->mWeights = new aiVertexWeight[(*it).second];

                bmap[(*it).first] = m->mNumBones++;
            }

            // .. and collect bone weights
            for (unsigned int i = 0,n = 0; i < m->mNumFaces; ++i) {
                TempTriangle& t = triangles[g.triangles[i]];

                for (unsigned int i = 0; i < 3; ++i,++n) {
                    const TempVertex& v = vertices[t.indices[i]];
                    for(unsigned int a = 0; a < 4; ++a) {
                        const unsigned int bone = v.bone_id[a];
                        if(bone==UINT_MAX){
                            continue;
                        }

                        aiBone* const outbone = m->mBones[bmap[bone]];
                        aiVertexWeight& outwght = outbone->mWeights[outbone->mNumWeights++];

                        outwght.mVertexId = n;
                        outwght.mWeight = v.weights[a];
                    }
                }
            }
        }
    }

    // ... add dummy nodes under a single root, each holding a reference to one
    // mesh. If we didn't do this, we'd lose the group name.
    aiNode* rt = pScene->mRootNode = new aiNode("<MS3DRoot>");

#ifdef ASSIMP_BUILD_MS3D_ONE_NODE_PER_MESH
    rt->mChildren = new aiNode*[rt->mNumChildren=pScene->mNumMeshes+(joints.size()?1:0)]();

    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        aiNode* nd = rt->mChildren[i] = new aiNode();

        const TempGroup& g = groups[i];

        // we need to generate an unique name for all mesh nodes.
        // since we want to keep the group name, a prefix is
        // prepended.
        nd->mName = aiString("<MS3DMesh>_");
        nd->mName.Append(g.name);
        nd->mParent = rt;

        nd->mMeshes = new unsigned int[nd->mNumMeshes = 1];
        nd->mMeshes[0] = i;
    }
#else
    rt->mMeshes = new unsigned int[pScene->mNumMeshes];
    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        rt->mMeshes[rt->mNumMeshes++] = i;
    }
#endif

    // convert animations as well
    if(joints.size()) {
#ifndef ASSIMP_BUILD_MS3D_ONE_NODE_PER_MESH
        rt->mChildren = new aiNode*[1]();
        rt->mNumChildren = 1;

        aiNode* jt = rt->mChildren[0] = new aiNode();
#else
        aiNode* jt = rt->mChildren[pScene->mNumMeshes] = new aiNode();
#endif
        jt->mParent = rt;
        CollectChildJoints(joints,jt);
        jt->mName.Set("<MS3DJointRoot>");

        pScene->mAnimations = new aiAnimation*[ pScene->mNumAnimations = 1 ];
        aiAnimation* const anim = pScene->mAnimations[0] = new aiAnimation();

        anim->mName.Set("<MS3DMasterAnim>");

        // carry the fps info to the user by scaling all times with it
        anim->mTicksPerSecond = animfps;

        // leave duration at its default, so ScenePreprocessor will fill an appropriate
        // value (the values taken from some MS3D files seem to be too unreliable
        // to pass the validation)
        // anim->mDuration = totalframes/animfps;

        anim->mChannels = new aiNodeAnim*[joints.size()]();
        for(std::vector<TempJoint>::const_iterator it = joints.begin(); it != joints.end(); ++it) {
            if ((*it).rotFrames.empty() && (*it).posFrames.empty()) {
                continue;
            }

            aiNodeAnim* nd = anim->mChannels[anim->mNumChannels++] = new aiNodeAnim();
            nd->mNodeName.Set((*it).name);

            if ((*it).rotFrames.size()) {
                nd->mRotationKeys = new aiQuatKey[(*it).rotFrames.size()];
                for(std::vector<TempKeyFrame>::const_iterator rot = (*it).rotFrames.begin(); rot != (*it).rotFrames.end(); ++rot) {
                    aiQuatKey& q = nd->mRotationKeys[nd->mNumRotationKeys++];

                    q.mTime = (*rot).time*animfps;
                    q.mValue = aiQuaternion(aiMatrix3x3(aiMatrix4x4().FromEulerAnglesXYZ((*it).rotation)*
                        aiMatrix4x4().FromEulerAnglesXYZ((*rot).value)));
                }
            }

            if ((*it).posFrames.size()) {
                nd->mPositionKeys = new aiVectorKey[(*it).posFrames.size()];

                aiQuatKey* qu = nd->mRotationKeys;
                for(std::vector<TempKeyFrame>::const_iterator pos = (*it).posFrames.begin(); pos != (*it).posFrames.end(); ++pos,++qu) {
                    aiVectorKey& v = nd->mPositionKeys[nd->mNumPositionKeys++];

                    v.mTime = (*pos).time*animfps;
                    v.mValue = (*it).position + (*pos).value;
                }
            }
        }
        // fixup to pass the validation if not a single animation channel is non-trivial
        if (!anim->mNumChannels) {
            anim->mChannels = NULL;
        }
    }
}

#endif
