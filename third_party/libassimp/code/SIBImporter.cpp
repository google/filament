/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team


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

/** @file  SIBImporter.cpp
 *  @brief Implementation of the SIB importer class.
 *
 *  The Nevercenter Silo SIB format is undocumented.
 *  All details here have been reverse engineered from
 *  studying the binary files output by Silo.
 *
 *  Nevertheless, this implementation is reasonably complete.
 */


#ifndef ASSIMP_BUILD_NO_SIB_IMPORTER

// internal headers
#include "SIBImporter.h"
#include "ByteSwapper.h"
#include "StreamReader.h"
#include "TinyFormatter.h"
//#include "../contrib/ConvertUTF/ConvertUTF.h"
#include "../contrib/utf8cpp/source/utf8.h"
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

#include <map>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Silo SIB Importer",
    "Richard Mitton (http://www.codersnotes.com/about)",
    "",
    "Does not apply subdivision.",
    aiImporterFlags_SupportBinaryFlavour,
    0, 0,
    0, 0,
    "sib"
};

struct SIBChunk {
    uint32_t    Tag;
    uint32_t    Size;
} PACK_STRUCT;

enum { 
    POS, 
    NRM, 
    UV,    
    N
};

typedef std::pair<uint32_t, uint32_t> SIBPair;

struct SIBEdge {
    uint32_t faceA, faceB;
    bool creased;
};

struct SIBMesh {
    aiMatrix4x4 axis;
    uint32_t numPts;
    std::vector<aiVector3D> pos, nrm, uv;
    std::vector<uint32_t> idx;
    std::vector<uint32_t> faceStart;
    std::vector<uint32_t> mtls;
    std::vector<SIBEdge> edges;
    std::map<SIBPair, uint32_t> edgeMap;
};

struct SIBObject {
    aiString name;
    aiMatrix4x4 axis;
    size_t meshIdx, meshCount;
};

struct SIB {
    std::vector<aiMaterial*> mtls;
    std::vector<aiMesh*> meshes;
    std::vector<aiLight*> lights;
    std::vector<SIBObject> objs, insts;
};

// ------------------------------------------------------------------------------------------------
static SIBEdge& GetEdge(SIBMesh* mesh, uint32_t posA, uint32_t posB) {
    SIBPair pair = (posA < posB) ? SIBPair(posA, posB) : SIBPair(posB, posA);
    std::map<SIBPair, uint32_t>::iterator it = mesh->edgeMap.find(pair);
    if (it != mesh->edgeMap.end())
        return mesh->edges[it->second];

    SIBEdge edge;
    edge.creased = false;
    edge.faceA = edge.faceB = 0xffffffff;
    mesh->edgeMap[pair] = static_cast<uint32_t>(mesh->edges.size());
    mesh->edges.push_back(edge);
    return mesh->edges.back();
}

// ------------------------------------------------------------------------------------------------
// Helpers for reading chunked data.

#define TAG(A,B,C,D) ((A << 24) | (B << 16) | (C << 8) | D)

static SIBChunk ReadChunk(StreamReaderLE* stream)
{
    SIBChunk chunk;
    chunk.Tag = stream->GetU4();
    chunk.Size = stream->GetU4();
    if (chunk.Size > stream->GetRemainingSizeToLimit())
        DefaultLogger::get()->error("SIB: Chunk overflow");
    ByteSwap::Swap4(&chunk.Tag);
    return chunk;
}

static aiColor3D ReadColor(StreamReaderLE* stream)
{
    float r = stream->GetF4();
    float g = stream->GetF4();
    float b = stream->GetF4();
    stream->GetU4(); // Colors have an unused(?) 4th component.
    return aiColor3D(r, g, b);
}

static void UnknownChunk(StreamReaderLE* /*stream*/, const SIBChunk& chunk)
{
    char temp[5] = {
        static_cast<char>(( chunk.Tag>>24 ) & 0xff),
        static_cast<char>(( chunk.Tag>>16 ) & 0xff),
        static_cast<char>(( chunk.Tag>>8 ) & 0xff),
        static_cast<char>(chunk.Tag & 0xff), '\0'
    };

    DefaultLogger::get()->warn((Formatter::format(), "SIB: Skipping unknown '",temp,"' chunk."));
}

// Reads a UTF-16LE string and returns it at UTF-8.
static aiString ReadString(StreamReaderLE *stream, uint32_t numWChars) {
    if ( nullptr == stream || 0 == numWChars ) {
        static const aiString empty;
        return empty;
    }

    // Allocate buffers (max expansion is 1 byte -> 4 bytes for UTF-8)
    std::vector<unsigned char> str;
    str.reserve( numWChars * 4 + 1 );
    uint16_t *temp = new uint16_t[ numWChars ];
    for ( uint32_t n = 0; n < numWChars; ++n ) {
        temp[ n ] = stream->GetU2();
    }

    // Convert it and NUL-terminate.
    const uint16_t *start( temp ), *end( temp + numWChars );
    utf8::utf16to8( start, end, back_inserter( str ) );
    str[ str.size() - 1 ] = '\0';

    // Return the final string.
    aiString result = aiString((const char *)&str[0]);
    delete[] temp;

    return result;
}

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
SIBImporter::SIBImporter() {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
SIBImporter::~SIBImporter() {
    // empty
}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool SIBImporter::CanRead( const std::string& pFile, IOSystem* /*pIOHandler*/, bool /*checkSig*/) const {
    return SimpleExtensionCheck(pFile, "sib");
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* SIBImporter::GetInfo () const {
    return &desc;
}

// ------------------------------------------------------------------------------------------------
static void ReadVerts(SIBMesh* mesh, StreamReaderLE* stream, uint32_t count) {
    if ( nullptr == mesh || nullptr == stream ) {
        return;
    }

    mesh->pos.resize(count);
    for ( uint32_t n=0; n<count; ++n ) {
        mesh->pos[ n ].x = stream->GetF4();
        mesh->pos[ n ].y = stream->GetF4();
        mesh->pos[ n ].z = stream->GetF4();
    }
}

// ------------------------------------------------------------------------------------------------
static void ReadFaces(SIBMesh* mesh, StreamReaderLE* stream)
{
    uint32_t ptIdx = 0;
    while (stream->GetRemainingSizeToLimit() > 0)
    {
        uint32_t numPoints = stream->GetU4();

        // Store room for the N index channels, plus the point count.
        size_t pos = mesh->idx.size() + 1;
        mesh->idx.resize(pos + numPoints*N);
        mesh->idx[pos-1] = numPoints;
        uint32_t *idx = &mesh->idx[pos];

        mesh->faceStart.push_back(static_cast<uint32_t>(pos-1));
        mesh->mtls.push_back(0);

        // Read all the position data.
        // UV/normals will be supplied later.
        // Positions are supplied indexed already, so we preserve that
        // mapping. UVs are supplied uniquely, so we allocate unique indices.
        for (uint32_t n=0;n<numPoints;n++,idx+=N,ptIdx++)
        {
            uint32_t p = stream->GetU4();
            if (p >= mesh->pos.size())
                throw DeadlyImportError("Vertex index is out of range.");
            idx[POS] = p;
            idx[NRM] = ptIdx;
            idx[UV] = ptIdx;
        }
    }

    // Allocate data channels for normals/UVs.
    mesh->nrm.resize(ptIdx, aiVector3D(0,0,0));
    mesh->uv.resize(ptIdx, aiVector3D(0,0,0));

    mesh->numPts = ptIdx;
}

// ------------------------------------------------------------------------------------------------
static void ReadUVs(SIBMesh* mesh, StreamReaderLE* stream)
{
    while (stream->GetRemainingSizeToLimit() > 0)
    {
        uint32_t faceIdx = stream->GetU4();
        uint32_t numPoints = stream->GetU4();

        if (faceIdx >= mesh->faceStart.size())
            throw DeadlyImportError("Invalid face index.");

        uint32_t pos = mesh->faceStart[faceIdx];
        uint32_t *idx = &mesh->idx[pos + 1];

        for (uint32_t n=0;n<numPoints;n++,idx+=N)
        {
            uint32_t id = idx[UV];
            mesh->uv[id].x = stream->GetF4();
            mesh->uv[id].y = stream->GetF4();
        }
    }
}

// ------------------------------------------------------------------------------------------------
static void ReadMtls(SIBMesh* mesh, StreamReaderLE* stream)
{
    // Material assignments are stored run-length encoded.
    // Also, we add 1 to each material so that we can use mtl #0
    // as the default material.
    uint32_t prevFace = stream->GetU4();
    uint32_t prevMtl = stream->GetU4() + 1;
    while (stream->GetRemainingSizeToLimit() > 0)
    {
        uint32_t face = stream->GetU4();
        uint32_t mtl = stream->GetU4() + 1;
        while (prevFace < face)
        {
            if (prevFace >= mesh->mtls.size())
                throw DeadlyImportError("Invalid face index.");
            mesh->mtls[prevFace++] = prevMtl;
        }

        prevFace = face;
        prevMtl = mtl;
    }

    while (prevFace < mesh->mtls.size())
        mesh->mtls[prevFace++] = prevMtl;
}

// ------------------------------------------------------------------------------------------------
static void ReadAxis(aiMatrix4x4& axis, StreamReaderLE* stream)
{
    axis.a4 = stream->GetF4();
    axis.b4 = stream->GetF4();
    axis.c4 = stream->GetF4();
    axis.d4 = 1;
    axis.a1 = stream->GetF4();
    axis.b1 = stream->GetF4();
    axis.c1 = stream->GetF4();
    axis.d1 = 0;
    axis.a2 = stream->GetF4();
    axis.b2 = stream->GetF4();
    axis.c2 = stream->GetF4();
    axis.d2 = 0;
    axis.a3 = stream->GetF4();
    axis.b3 = stream->GetF4();
    axis.c3 = stream->GetF4();
    axis.d3 = 0;
}

// ------------------------------------------------------------------------------------------------
static void ReadEdges(SIBMesh* mesh, StreamReaderLE* stream)
{
    while (stream->GetRemainingSizeToLimit() > 0)
    {
        uint32_t posA = stream->GetU4();
        uint32_t posB = stream->GetU4();
        GetEdge(mesh, posA, posB);
    }
}

// ------------------------------------------------------------------------------------------------
static void ReadCreases(SIBMesh* mesh, StreamReaderLE* stream)
{
    while (stream->GetRemainingSizeToLimit() > 0)
    {
        uint32_t edge = stream->GetU4();
        if (edge >= mesh->edges.size())
            throw DeadlyImportError("SIB: Invalid edge index.");
        mesh->edges[edge].creased = true;
    }
}

// ------------------------------------------------------------------------------------------------
static void ConnectFaces(SIBMesh* mesh)
{
    // Find faces connected to each edge.
    size_t numFaces = mesh->faceStart.size();
    for (size_t faceIdx=0;faceIdx<numFaces;faceIdx++)
    {
        uint32_t *idx = &mesh->idx[mesh->faceStart[faceIdx]];
        uint32_t numPoints = *idx++;
        uint32_t prev = idx[(numPoints-1)*N+POS];

        for (uint32_t i=0;i<numPoints;i++,idx+=N)
        {
            uint32_t next = idx[POS];

            // Find this edge.
            SIBEdge& edge = GetEdge(mesh, prev, next);

            // Link this face onto it.
            // This gives potentially undesirable normals when used
            // with non-2-manifold surfaces, but then so does Silo to begin with.
            if (edge.faceA == 0xffffffff)
                edge.faceA = static_cast<uint32_t>(faceIdx);
            else if (edge.faceB == 0xffffffff)
                edge.faceB = static_cast<uint32_t>(faceIdx);

            prev = next;
        }
    }
}

// ------------------------------------------------------------------------------------------------
static aiVector3D CalculateVertexNormal(SIBMesh* mesh, uint32_t faceIdx, uint32_t pos,
                                        const std::vector<aiVector3D>& faceNormals)
{
    // Creased edges complicate this. We need to find the start/end range of the
    // ring of faces that touch this position.
    // We do this in two passes. The first pass is to find the end of the range,
    // the second is to work backwards to the start and calculate the final normal.
    aiVector3D vtxNormal;
    for (int pass=0;pass<2;pass++)
    {
        vtxNormal = aiVector3D(0, 0, 0);
        uint32_t startFaceIdx = faceIdx;
        uint32_t prevFaceIdx = faceIdx;

        // Process each connected face.
        while (true)
        {
            // Accumulate the face normal.
            vtxNormal += faceNormals[faceIdx];

            uint32_t nextFaceIdx = 0xffffffff;

            // Move to the next edge sharing this position.
            uint32_t* idx = &mesh->idx[mesh->faceStart[faceIdx]];
            uint32_t numPoints = *idx++;
            uint32_t posA = idx[(numPoints-1)*N+POS];
            for (uint32_t n=0;n<numPoints;n++,idx+=N)
            {
                uint32_t posB = idx[POS];

                // Test if this edge shares our target position.
                if (posA == pos || posB == pos)
                {
                    SIBEdge& edge = GetEdge(mesh, posA, posB);

                    // Non-manifold meshes can produce faces which share
                    // positions but have no edge entry, so check it.
                    if (edge.faceA == faceIdx || edge.faceB == faceIdx)
                    {
                        // Move to whichever side we didn't just come from.
                        if (!edge.creased) {
                            if (edge.faceA != prevFaceIdx && edge.faceA != faceIdx && edge.faceA != 0xffffffff)
                                nextFaceIdx = edge.faceA;
                            else if (edge.faceB != prevFaceIdx && edge.faceB != faceIdx && edge.faceB != 0xffffffff)
                                nextFaceIdx = edge.faceB;
                        }
                    }
                }

                posA = posB;
            }

            // Stop once we hit either an creased/unconnected edge, or we
            // wrapped around and hit our start point.
            if (nextFaceIdx == 0xffffffff || nextFaceIdx == startFaceIdx)
                break;

            prevFaceIdx = faceIdx;
            faceIdx = nextFaceIdx;
        }
    }

    // Normalize it.
    float len = vtxNormal.Length();
    if (len > 0.000000001f)
        vtxNormal /= len;
    return vtxNormal;
}

// ------------------------------------------------------------------------------------------------
static void CalculateNormals(SIBMesh* mesh)
{
    size_t numFaces = mesh->faceStart.size();

    // Calculate face normals.
    std::vector<aiVector3D> faceNormals(numFaces);
    for (size_t faceIdx=0;faceIdx<numFaces;faceIdx++)
    {
        uint32_t* idx = &mesh->idx[mesh->faceStart[faceIdx]];
        uint32_t numPoints = *idx++;

        aiVector3D faceNormal(0, 0, 0);

        uint32_t *prev = &idx[(numPoints-1)*N];

        for (uint32_t i=0;i<numPoints;i++)
        {
            uint32_t *next = &idx[i*N];

            faceNormal += mesh->pos[prev[POS]] ^ mesh->pos[next[POS]];
            prev = next;
        }

        faceNormals[faceIdx] = faceNormal;
    }

    // Calculate vertex normals.
    for (size_t faceIdx=0;faceIdx<numFaces;faceIdx++)
    {
        uint32_t* idx = &mesh->idx[mesh->faceStart[faceIdx]];
        uint32_t numPoints = *idx++;

        for (uint32_t i=0;i<numPoints;i++)
        {
            uint32_t pos = idx[i*N+POS];
            uint32_t nrm = idx[i*N+NRM];
            aiVector3D vtxNorm = CalculateVertexNormal(mesh, static_cast<uint32_t>(faceIdx), pos, faceNormals);
            mesh->nrm[nrm] = vtxNorm;
        }
    }
}

// ------------------------------------------------------------------------------------------------
struct TempMesh
{
    std::vector<aiVector3D> vtx;
    std::vector<aiVector3D> nrm;
    std::vector<aiVector3D> uv;
    std::vector<aiFace>     faces;
};

static void ReadShape(SIB* sib, StreamReaderLE* stream)
{
    SIBMesh smesh;
    aiString name;

    while (stream->GetRemainingSizeToLimit() >= sizeof(SIBChunk))
    {
        SIBChunk chunk = ReadChunk(stream);
        unsigned oldLimit = stream->SetReadLimit(stream->GetCurrentPos() + chunk.Size);

        switch (chunk.Tag)
        {
        case TAG('M','I','R','P'): break; // mirror plane maybe?
        case TAG('I','M','R','P'): break; // instance mirror? (not supported here yet)
        case TAG('D','I','N','F'): break; // display info, not needed
        case TAG('P','I','N','F'): break; // ?
        case TAG('V','M','I','R'): break; // ?
        case TAG('F','M','I','R'): break; // ?
        case TAG('T','X','S','M'): break; // ?
        case TAG('F','A','H','S'): break; // ?
        case TAG('V','R','T','S'): ReadVerts(&smesh, stream, chunk.Size/12); break;
        case TAG('F','A','C','S'): ReadFaces(&smesh, stream); break;
        case TAG('F','T','V','S'): ReadUVs(&smesh, stream); break;
        case TAG('S','N','A','M'): name = ReadString(stream, chunk.Size/2); break;
        case TAG('F','A','M','A'): ReadMtls(&smesh, stream); break;
        case TAG('A','X','I','S'): ReadAxis(smesh.axis, stream); break;
        case TAG('E','D','G','S'): ReadEdges(&smesh, stream); break;
        case TAG('E','C','R','S'): ReadCreases(&smesh, stream); break;
        default:                   UnknownChunk(stream, chunk); break;
        }

        stream->SetCurrentPos(stream->GetReadLimit());
        stream->SetReadLimit(oldLimit);
    }

    ai_assert(smesh.faceStart.size() == smesh.mtls.size()); // sanity check

    // Silo doesn't store any normals in the file - we need to compute
    // them ourselves. We can't let AssImp handle it as AssImp doesn't
    // know about our creased edges.
    ConnectFaces(&smesh);
    CalculateNormals(&smesh);

    // Construct the transforms.
    aiMatrix4x4 worldToLocal = smesh.axis;
    worldToLocal.Inverse();
    aiMatrix4x4 worldToLocalN = worldToLocal;
    worldToLocalN.a4 = worldToLocalN.b4 = worldToLocalN.c4 = 0.0f;
    worldToLocalN.Inverse().Transpose();

    // Allocate final mesh data.
    // We'll allocate one mesh for each material. (we'll strip unused ones after)
    std::vector<TempMesh> meshes(sib->mtls.size());

    // Un-index the source data and apply to each vertex.
    for (unsigned fi=0;fi<smesh.faceStart.size();fi++)
    {
        uint32_t start = smesh.faceStart[fi];
        uint32_t mtl = smesh.mtls[fi];
        uint32_t *idx = &smesh.idx[start];

        if (mtl >= meshes.size())
        {
            DefaultLogger::get()->error("SIB: Face material index is invalid.");
            mtl = 0;
        }

        TempMesh& dest = meshes[mtl];

        aiFace face;
        face.mNumIndices = *idx++;
        face.mIndices = new unsigned[face.mNumIndices];
        for (unsigned pt=0;pt<face.mNumIndices;pt++,idx+=N)
        {
            size_t vtxIdx = dest.vtx.size();
            face.mIndices[pt] = static_cast<unsigned int>(vtxIdx);

            // De-index it. We don't need to validate here as
            // we did it when creating the data.
            aiVector3D pos = smesh.pos[idx[POS]];
            aiVector3D nrm = smesh.nrm[idx[NRM]];
            aiVector3D uv  = smesh.uv[idx[UV]];

            // The verts are supplied in world-space, so let's
            // transform them back into the local space of this mesh:
            pos = worldToLocal * pos;
            nrm = worldToLocalN * nrm;

            dest.vtx.push_back(pos);
            dest.nrm.push_back(nrm);
            dest.uv.push_back(uv);
        }
        dest.faces.push_back(face);
    }

    SIBObject obj;
    obj.name = name;
    obj.axis = smesh.axis;
    obj.meshIdx = sib->meshes.size();

    // Now that we know the size of everything,
    // we can build the final one-material-per-mesh data.
    for (size_t n=0;n<meshes.size();n++)
    {
        TempMesh& src = meshes[n];
        if (src.faces.empty())
            continue;

        aiMesh* mesh = new aiMesh;
        mesh->mName = name;
        mesh->mNumFaces = static_cast<unsigned int>(src.faces.size());
        mesh->mFaces = new aiFace[mesh->mNumFaces];
        mesh->mNumVertices = static_cast<unsigned int>(src.vtx.size());
        mesh->mVertices = new aiVector3D[mesh->mNumVertices];
        mesh->mNormals = new aiVector3D[mesh->mNumVertices];
        mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
        mesh->mNumUVComponents[0] = 2;
        mesh->mMaterialIndex = static_cast<unsigned int>(n);

        for (unsigned i=0;i<mesh->mNumVertices;i++)
        {
            mesh->mVertices[i] = src.vtx[i];
            mesh->mNormals[i] = src.nrm[i];
            mesh->mTextureCoords[0][i] = src.uv[i];
        }
        for (unsigned i=0;i<mesh->mNumFaces;i++)
        {
            mesh->mFaces[i] = src.faces[i];
        }

        sib->meshes.push_back(mesh);
    }

    obj.meshCount = sib->meshes.size() - obj.meshIdx;
    sib->objs.push_back(obj);
}

// ------------------------------------------------------------------------------------------------
static void ReadMaterial(SIB* sib, StreamReaderLE* stream)
{
    aiColor3D diff = ReadColor(stream);
    aiColor3D ambi = ReadColor(stream);
    aiColor3D spec = ReadColor(stream);
    aiColor3D emis = ReadColor(stream);
    float shiny = (float)stream->GetU4();

    uint32_t nameLen = stream->GetU4();
    aiString name = ReadString(stream, nameLen/2);
    uint32_t texLen = stream->GetU4();
    aiString tex = ReadString(stream, texLen/2);

    aiMaterial* mtl = new aiMaterial();
    mtl->AddProperty(&diff, 1, AI_MATKEY_COLOR_DIFFUSE);
    mtl->AddProperty(&ambi, 1, AI_MATKEY_COLOR_AMBIENT);
    mtl->AddProperty(&spec, 1, AI_MATKEY_COLOR_SPECULAR);
    mtl->AddProperty(&emis, 1, AI_MATKEY_COLOR_EMISSIVE);
    mtl->AddProperty(&shiny, 1, AI_MATKEY_SHININESS);
    mtl->AddProperty(&name, AI_MATKEY_NAME);
    if (tex.length > 0) {
        mtl->AddProperty(&tex, AI_MATKEY_TEXTURE_DIFFUSE(0));
        mtl->AddProperty(&tex, AI_MATKEY_TEXTURE_AMBIENT(0));
    }

    sib->mtls.push_back(mtl);
}

// ------------------------------------------------------------------------------------------------
static void ReadLightInfo(aiLight* light, StreamReaderLE* stream)
{
    uint32_t type = stream->GetU4();
    switch (type) {
    case 0: light->mType = aiLightSource_POINT; break;
    case 1: light->mType = aiLightSource_SPOT; break;
    case 2: light->mType = aiLightSource_DIRECTIONAL; break;
    default: light->mType = aiLightSource_UNDEFINED; break;
    }

    light->mPosition.x = stream->GetF4();
    light->mPosition.y = stream->GetF4();
    light->mPosition.z = stream->GetF4();
    light->mDirection.x = stream->GetF4();
    light->mDirection.y = stream->GetF4();
    light->mDirection.z = stream->GetF4();
    light->mColorDiffuse = ReadColor(stream);
    light->mColorAmbient = ReadColor(stream);
    light->mColorSpecular = ReadColor(stream);
    ai_real spotExponent = stream->GetF4();
    ai_real spotCutoff = stream->GetF4();
    light->mAttenuationConstant = stream->GetF4();
    light->mAttenuationLinear = stream->GetF4();
    light->mAttenuationQuadratic = stream->GetF4();

    // Silo uses the OpenGL default lighting model for it's
    // spot cutoff/exponent. AssImp unfortunately, does not.
    // Let's try and approximate it by solving for the
    // 99% and 1% percentiles.
    //    OpenGL: I = cos(angle)^E
    //   Solving: angle = acos(I^(1/E))
    ai_real E = ai_real( 1.0 ) / std::max(spotExponent, (ai_real)0.00001);
    ai_real inner = std::acos(std::pow((ai_real)0.99, E));
    ai_real outer = std::acos(std::pow((ai_real)0.01, E));

    // Apply the cutoff.
    outer = std::min(outer, AI_DEG_TO_RAD(spotCutoff));

    light->mAngleInnerCone = std::min(inner, outer);
    light->mAngleOuterCone = outer;
}

static void ReadLight(SIB* sib, StreamReaderLE* stream)
{
    aiLight* light = new aiLight();

    while (stream->GetRemainingSizeToLimit() >= sizeof(SIBChunk))
    {
        SIBChunk chunk = ReadChunk(stream);
        unsigned oldLimit = stream->SetReadLimit(stream->GetCurrentPos() + chunk.Size);

        switch (chunk.Tag)
        {
        case TAG('L','N','F','O'): ReadLightInfo(light, stream); break;
        case TAG('S','N','A','M'): light->mName = ReadString(stream, chunk.Size/2); break;
        default:                   UnknownChunk(stream, chunk); break;
        }

        stream->SetCurrentPos(stream->GetReadLimit());
        stream->SetReadLimit(oldLimit);
    }

    sib->lights.push_back(light);
}

// ------------------------------------------------------------------------------------------------
static void ReadScale(aiMatrix4x4& axis, StreamReaderLE* stream)
{
    aiMatrix4x4 scale;
    scale.a1 = stream->GetF4();
    scale.b1 = stream->GetF4();
    scale.c1 = stream->GetF4();
    scale.d1 = stream->GetF4();
    scale.a2 = stream->GetF4();
    scale.b2 = stream->GetF4();
    scale.c2 = stream->GetF4();
    scale.d2 = stream->GetF4();
    scale.a3 = stream->GetF4();
    scale.b3 = stream->GetF4();
    scale.c3 = stream->GetF4();
    scale.d3 = stream->GetF4();
    scale.a4 = stream->GetF4();
    scale.b4 = stream->GetF4();
    scale.c4 = stream->GetF4();
    scale.d4 = stream->GetF4();

    axis = axis * scale;
}

static void ReadInstance(SIB* sib, StreamReaderLE* stream)
{
    SIBObject inst;
    uint32_t shapeIndex = 0;

    while (stream->GetRemainingSizeToLimit() >= sizeof(SIBChunk))
    {
        SIBChunk chunk = ReadChunk(stream);
        unsigned oldLimit = stream->SetReadLimit(stream->GetCurrentPos() + chunk.Size);

        switch (chunk.Tag)
        {
        case TAG('D','I','N','F'): break; // display info, not needed
        case TAG('P','I','N','F'): break; // ?
        case TAG('A','X','I','S'): ReadAxis(inst.axis, stream); break;
        case TAG('I','N','S','I'): shapeIndex = stream->GetU4(); break;
        case TAG('S','M','T','X'): ReadScale(inst.axis, stream); break;
        case TAG('S','N','A','M'): inst.name = ReadString(stream, chunk.Size/2); break;
        default:                   UnknownChunk(stream, chunk); break;
        }

        stream->SetCurrentPos(stream->GetReadLimit());
        stream->SetReadLimit(oldLimit);
    }

    if ( shapeIndex >= sib->objs.size() ) {
        throw DeadlyImportError( "SIB: Invalid shape index." );
    }

    const SIBObject& src = sib->objs[shapeIndex];
    inst.meshIdx = src.meshIdx;
    inst.meshCount = src.meshCount;
    sib->insts.push_back(inst);
}

// ------------------------------------------------------------------------------------------------
static void CheckVersion(StreamReaderLE* stream)
{
    uint32_t version = stream->GetU4();
    if ( version < 1 || version > 2 ) {
        throw DeadlyImportError( "SIB: Unsupported file version." );
    }
}

static void ReadScene(SIB* sib, StreamReaderLE* stream)
{
    // Parse each chunk in turn.
    while (stream->GetRemainingSizeToLimit() >= sizeof(SIBChunk))
    {
        SIBChunk chunk = ReadChunk(stream);
        unsigned oldLimit = stream->SetReadLimit(stream->GetCurrentPos() + chunk.Size);

        switch (chunk.Tag)
        {
        case TAG('H','E','A','D'): CheckVersion(stream); break;
        case TAG('S','H','A','P'): ReadShape(sib, stream); break;
        case TAG('G','R','P','S'): break; // group assignment, we don't import this
        case TAG('T','E','X','P'): break; // ?
        case TAG('I','N','S','T'): ReadInstance(sib, stream); break;
        case TAG('M','A','T','R'): ReadMaterial(sib, stream); break;
        case TAG('L','G','H','T'): ReadLight(sib, stream); break;
        default:                   UnknownChunk(stream, chunk); break;
        }

        stream->SetCurrentPos(stream->GetReadLimit());
        stream->SetReadLimit(oldLimit);
    }
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void SIBImporter::InternReadFile(const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    StreamReaderLE stream(pIOHandler->Open(pFile, "rb"));

    // We should have at least one chunk
    if (stream.GetRemainingSize() < 16)
        throw DeadlyImportError("SIB file is either empty or corrupt: " + pFile);

    SIB sib;

    // Default material.
    aiMaterial* defmtl = new aiMaterial;
    aiString defname = aiString(AI_DEFAULT_MATERIAL_NAME);
    defmtl->AddProperty(&defname, AI_MATKEY_NAME);
    sib.mtls.push_back(defmtl);

    // Read it all.
    ReadScene(&sib, &stream);

    // Join the instances and objects together.
    size_t firstInst = sib.objs.size();
    sib.objs.insert(sib.objs.end(), sib.insts.begin(), sib.insts.end());
    sib.insts.clear();

    // Transfer to the aiScene.
    pScene->mNumMaterials = static_cast<unsigned int>(sib.mtls.size());
    pScene->mNumMeshes = static_cast<unsigned int>(sib.meshes.size());
    pScene->mNumLights = static_cast<unsigned int>(sib.lights.size());
    pScene->mMaterials = pScene->mNumMaterials ? new aiMaterial*[pScene->mNumMaterials] : NULL;
    pScene->mMeshes = pScene->mNumMeshes ? new aiMesh*[pScene->mNumMeshes] : NULL;
    pScene->mLights = pScene->mNumLights ? new aiLight*[pScene->mNumLights] : NULL;
    if (pScene->mNumMaterials)
        memcpy(pScene->mMaterials, &sib.mtls[0], sizeof(aiMaterial*) * pScene->mNumMaterials);
    if (pScene->mNumMeshes)
        memcpy(pScene->mMeshes, &sib.meshes[0], sizeof(aiMesh*) * pScene->mNumMeshes);
    if (pScene->mNumLights)
        memcpy(pScene->mLights, &sib.lights[0], sizeof(aiLight*) * pScene->mNumLights);

    // Construct the root node.
    size_t childIdx = 0;
    aiNode *root = new aiNode();
    root->mName.Set("<SIBRoot>");
    root->mNumChildren = static_cast<unsigned int>(sib.objs.size() + sib.lights.size());
    root->mChildren = root->mNumChildren ? new aiNode*[root->mNumChildren] : NULL;
    pScene->mRootNode = root;

    // Add nodes for each object.
    for (size_t n=0;n<sib.objs.size();n++)
    {
        ai_assert(root->mChildren);
        SIBObject& obj = sib.objs[n];
        aiNode* node = new aiNode;
        root->mChildren[childIdx++] = node;
        node->mName = obj.name;
        node->mParent = root;
        node->mTransformation = obj.axis;

        node->mNumMeshes = static_cast<unsigned int>(obj.meshCount);
        node->mMeshes = node->mNumMeshes ? new unsigned[node->mNumMeshes] : NULL;
        for (unsigned i=0;i<node->mNumMeshes;i++)
            node->mMeshes[i] = static_cast<unsigned int>(obj.meshIdx + i);

        // Mark instanced objects as being so.
        if (n >= firstInst)
        {
            node->mMetaData = aiMetadata::Alloc( 1 );
            node->mMetaData->Set( 0, "IsInstance", true );
        }
    }

    // Add nodes for each light.
    // (no transformation as the light is already in world space)
    for (size_t n=0;n<sib.lights.size();n++)
    {
        ai_assert(root->mChildren);
        aiLight* light = sib.lights[n];
        if ( nullptr != light ) {
            aiNode* node = new aiNode;
            root->mChildren[ childIdx++ ] = node;
            node->mName = light->mName;
            node->mParent = root;
        }
    }
}

#endif // !! ASSIMP_BUILD_NO_SIB_IMPORTER
