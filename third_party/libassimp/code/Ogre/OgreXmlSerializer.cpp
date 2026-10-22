/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


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

#include "OgreXmlSerializer.h"
#include "OgreBinarySerializer.h"
#include "OgreParsingUtils.h"

#include <assimp/TinyFormatter.h>
#include <assimp/DefaultLogger.hpp>
#include <memory>

#ifndef ASSIMP_BUILD_NO_OGRE_IMPORTER

// Define as 1 to get verbose logging.
#define OGRE_XML_SERIALIZER_DEBUG 0

namespace Assimp
{
namespace Ogre
{

AI_WONT_RETURN void ThrowAttibuteError(const XmlReader* reader, const std::string &name, const std::string &error = "") AI_WONT_RETURN_SUFFIX;
AI_WONT_RETURN void ThrowAttibuteError(const XmlReader* reader, const std::string &name, const std::string &error)
{
    if (!error.empty())
    {
        throw DeadlyImportError(error + " in node '" + std::string(reader->getNodeName()) + "' and attribute '" + name + "'");
    }
    else
    {
        throw DeadlyImportError("Attribute '" + name + "' does not exist in node '" + std::string(reader->getNodeName()) + "'");
    }
}

template<>
int32_t OgreXmlSerializer::ReadAttribute<int32_t>(const char *name) const
{
    if (HasAttribute(name))
    {
        return static_cast<int32_t>(m_reader->getAttributeValueAsInt(name));
    }
    else
    {
        ThrowAttibuteError(m_reader, name);
        return 0;
    }
}

template<>
uint32_t OgreXmlSerializer::ReadAttribute<uint32_t>(const char *name) const
{
    if (HasAttribute(name))
    {
        /** @note This is hackish. But we are never expecting unsigned values that go outside the
            int32_t range. Just monitor for negative numbers and kill the import. */
        int32_t temp = ReadAttribute<int32_t>(name);
        if (temp >= 0)
        {
            return static_cast<uint32_t>(temp);
        }
        else
        {
            ThrowAttibuteError(m_reader, name, "Found a negative number value where expecting a uint32_t value");
        }
    }
    else
    {
        ThrowAttibuteError(m_reader, name);
    }
    return 0;
}

template<>
uint16_t OgreXmlSerializer::ReadAttribute<uint16_t>(const char *name) const
{
    if (HasAttribute(name))
    {
        return static_cast<uint16_t>(ReadAttribute<uint32_t>(name));
    }
    else
    {
        ThrowAttibuteError(m_reader, name);
    }
    return 0;
}

template<>
float OgreXmlSerializer::ReadAttribute<float>(const char *name) const
{
    if (HasAttribute(name))
    {
        return m_reader->getAttributeValueAsFloat(name);
    }
    else
    {
        ThrowAttibuteError(m_reader, name);
        return 0;
    }
}

template<>
std::string OgreXmlSerializer::ReadAttribute<std::string>(const char *name) const
{
    const char* value = m_reader->getAttributeValue(name);
    if (value)
    {
        return std::string(value);
    }
    else
    {
        ThrowAttibuteError(m_reader, name);
        return "";
    }
}

template<>
bool OgreXmlSerializer::ReadAttribute<bool>(const char *name) const
{
    std::string value = Ogre::ToLower(ReadAttribute<std::string>(name));
    if (ASSIMP_stricmp(value, "true") == 0)
    {
        return true;
    }
    else if (ASSIMP_stricmp(value, "false") == 0)
    {
        return false;
    }
    else
    {
        ThrowAttibuteError(m_reader, name, "Boolean value is expected to be 'true' or 'false', encountered '" + value + "'");
        return false;
    }
}

bool OgreXmlSerializer::HasAttribute(const char *name) const
{
    return (m_reader->getAttributeValue(name) != 0);
}

std::string &OgreXmlSerializer::NextNode()
{
    do
    {
        if (!m_reader->read())
        {
            m_currentNodeName = "";
            return m_currentNodeName;
        }
    }
    while(m_reader->getNodeType() != irr::io::EXN_ELEMENT);

    CurrentNodeName(true);
#if (OGRE_XML_SERIALIZER_DEBUG == 1)
    ASSIMP_LOG_DEBUG"<" + m_currentNodeName + ">");
#endif
    return m_currentNodeName;
}

bool OgreXmlSerializer::CurrentNodeNameEquals(const std::string &name) const
{
    return (ASSIMP_stricmp(m_currentNodeName, name) == 0);
}

std::string OgreXmlSerializer::CurrentNodeName(bool forceRead)
{
    if (forceRead)
        m_currentNodeName = std::string(m_reader->getNodeName());
    return m_currentNodeName;
}

std::string &OgreXmlSerializer::SkipCurrentNode()
{
#if (OGRE_XML_SERIALIZER_DEBUG == 1)
    ASSIMP_LOG_DEBUG("Skipping node <" + m_currentNodeName + ">");
#endif

    for(;;) {
        if (!m_reader->read()) {
            m_currentNodeName = "";
            return m_currentNodeName;
        }
        if ( m_reader->getNodeType() != irr::io::EXN_ELEMENT_END ) {
            continue;
        } else if ( std::string( m_reader->getNodeName() ) == m_currentNodeName ) {
            break;
        }
    }

    return NextNode();
}

// Mesh XML constants

// <mesh>
static const char *nnMesh                = "mesh";
static const char *nnSharedGeometry      = "sharedgeometry";
static const char *nnSubMeshes           = "submeshes";
static const char *nnSubMesh             = "submesh";
static const char *nnSubMeshNames        = "submeshnames";
static const char *nnSkeletonLink        = "skeletonlink";
static const char *nnLOD                 = "levelofdetail";
static const char *nnExtremes            = "extremes";
static const char *nnPoses               = "poses";
static const char *nnAnimations          = "animations";

// <submesh>
static const char *nnFaces               = "faces";
static const char *nnFace                = "face";
static const char *nnGeometry            = "geometry";
static const char *nnTextures            = "textures";

// <mesh/submesh>
static const char *nnBoneAssignments     = "boneassignments";

// <sharedgeometry/geometry>
static const char *nnVertexBuffer        = "vertexbuffer";

// <vertexbuffer>
static const char *nnVertex              = "vertex";
static const char *nnPosition            = "position";
static const char *nnNormal              = "normal";
static const char *nnTangent             = "tangent";
static const char *nnBinormal            = "binormal";
static const char *nnTexCoord            = "texcoord";
static const char *nnColorDiffuse        = "colour_diffuse";
static const char *nnColorSpecular       = "colour_specular";

// <boneassignments>
static const char *nnVertexBoneAssignment = "vertexboneassignment";

// Skeleton XML constants

// <skeleton>
static const char *nnSkeleton            = "skeleton";
static const char *nnBones               = "bones";
static const char *nnBoneHierarchy       = "bonehierarchy";
static const char *nnAnimationLinks      = "animationlinks";

// <bones>
static const char *nnBone                = "bone";
static const char *nnRotation            = "rotation";
static const char *nnAxis                = "axis";
static const char *nnScale               = "scale";

// <bonehierarchy>
static const char *nnBoneParent          = "boneparent";

// <animations>
static const char *nnAnimation           = "animation";
static const char *nnTracks              = "tracks";

// <tracks>
static const char *nnTrack               = "track";
static const char *nnKeyFrames           = "keyframes";
static const char *nnKeyFrame            = "keyframe";
static const char *nnTranslate           = "translate";
static const char *nnRotate              = "rotate";

// Common XML constants

static const char *anX = "x";
static const char *anY = "y";
static const char *anZ = "z";

// Mesh

MeshXml *OgreXmlSerializer::ImportMesh(XmlReader *reader) {
    OgreXmlSerializer serializer(reader);

    MeshXml *mesh = new MeshXml();
    serializer.ReadMesh(mesh);

    return mesh;
}

void OgreXmlSerializer::ReadMesh(MeshXml *mesh) {
    if (NextNode() != nnMesh) {
        throw DeadlyImportError("Root node is <" + m_currentNodeName + "> expecting <mesh>");
    }

    ASSIMP_LOG_DEBUG("Reading Mesh");

    NextNode();

    // Root level nodes
    while(m_currentNodeName == nnSharedGeometry  ||
          m_currentNodeName == nnSubMeshes       ||
          m_currentNodeName == nnSkeletonLink    ||
          m_currentNodeName == nnBoneAssignments ||
          m_currentNodeName == nnLOD             ||
          m_currentNodeName == nnSubMeshNames    ||
          m_currentNodeName == nnExtremes        ||
          m_currentNodeName == nnPoses           ||
          m_currentNodeName == nnAnimations)
    {
        if (m_currentNodeName == nnSharedGeometry)
        {
            mesh->sharedVertexData = new VertexDataXml();
            ReadGeometry(mesh->sharedVertexData);
        }
        else if (m_currentNodeName == nnSubMeshes)
        {
            NextNode();
            while(m_currentNodeName == nnSubMesh) {
                ReadSubMesh(mesh);
            }
        }
        else if (m_currentNodeName == nnBoneAssignments)
        {
            ReadBoneAssignments(mesh->sharedVertexData);
        }
        else if (m_currentNodeName == nnSkeletonLink)
        {
            mesh->skeletonRef = ReadAttribute<std::string>("name");
            ASSIMP_LOG_DEBUG_F("Read skeleton link ", mesh->skeletonRef);
            NextNode();
        }
        // Assimp incompatible/ignored nodes
        else
            SkipCurrentNode();
    }
}

void OgreXmlSerializer::ReadGeometry(VertexDataXml *dest)
{
    dest->count = ReadAttribute<uint32_t>("vertexcount");
    ASSIMP_LOG_DEBUG_F( "  - Reading geometry of ", dest->count, " vertices");

    NextNode();
    while(m_currentNodeName == nnVertexBuffer) {
        ReadGeometryVertexBuffer(dest);
    }
}

void OgreXmlSerializer::ReadGeometryVertexBuffer(VertexDataXml *dest)
{
    bool positions = (HasAttribute("positions") && ReadAttribute<bool>("positions"));
    bool normals   = (HasAttribute("normals") && ReadAttribute<bool>("normals"));
    bool tangents  = (HasAttribute("tangents") && ReadAttribute<bool>("tangents"));
    uint32_t uvs   = (HasAttribute("texture_coords") ? ReadAttribute<uint32_t>("texture_coords") : 0);

    // Not having positions is a error only if a previous vertex buffer did not have them.
    if (!positions && !dest->HasPositions()) {
        throw DeadlyImportError("Vertex buffer does not contain positions!");
    }

    if (positions)
    {
        ASSIMP_LOG_DEBUG("    - Contains positions");
        dest->positions.reserve(dest->count);
    }
    if (normals)
    {
        ASSIMP_LOG_DEBUG("    - Contains normals");
        dest->normals.reserve(dest->count);
    }
    if (tangents)
    {
        ASSIMP_LOG_DEBUG("    - Contains tangents");
        dest->tangents.reserve(dest->count);
    }
    if (uvs > 0)
    {
        ASSIMP_LOG_DEBUG_F( "    - Contains ", uvs, " texture coords");
        dest->uvs.resize(uvs);
        for(size_t i=0, len=dest->uvs.size(); i<len; ++i) {
            dest->uvs[i].reserve(dest->count);
        }
    }

    bool warnBinormal = true;
    bool warnColorDiffuse = true;
    bool warnColorSpecular = true;

    NextNode();

    while(m_currentNodeName == nnVertex       ||
          m_currentNodeName == nnPosition     ||
          m_currentNodeName == nnNormal       ||
          m_currentNodeName == nnTangent      ||
          m_currentNodeName == nnBinormal     ||
          m_currentNodeName == nnTexCoord     ||
          m_currentNodeName == nnColorDiffuse ||
          m_currentNodeName == nnColorSpecular)
    {
        if (m_currentNodeName == nnVertex) {
            NextNode();
        }

        /// @todo Implement nnBinormal, nnColorDiffuse and nnColorSpecular

        if (positions && m_currentNodeName == nnPosition)
        {
            aiVector3D pos;
            pos.x = ReadAttribute<float>(anX);
            pos.y = ReadAttribute<float>(anY);
            pos.z = ReadAttribute<float>(anZ);
            dest->positions.push_back(pos);
        }
        else if (normals && m_currentNodeName == nnNormal)
        {
            aiVector3D normal;
            normal.x = ReadAttribute<float>(anX);
            normal.y = ReadAttribute<float>(anY);
            normal.z = ReadAttribute<float>(anZ);
            dest->normals.push_back(normal);
        }
        else if (tangents && m_currentNodeName == nnTangent)
        {
            aiVector3D tangent;
            tangent.x = ReadAttribute<float>(anX);
            tangent.y = ReadAttribute<float>(anY);
            tangent.z = ReadAttribute<float>(anZ);
            dest->tangents.push_back(tangent);
        }
        else if (uvs > 0 && m_currentNodeName == nnTexCoord)
        {
            for(auto &uvs : dest->uvs)
            {
                if (m_currentNodeName != nnTexCoord) {
                    throw DeadlyImportError("Vertex buffer declared more UVs than can be found in a vertex");
                }

                aiVector3D uv;
                uv.x = ReadAttribute<float>("u");
                uv.y = (ReadAttribute<float>("v") * -1) + 1; // Flip UV from Ogre to Assimp form
                uvs.push_back(uv);

                NextNode();
            }
            // Continue main loop as above already read next node
            continue;
        }
        else
        {
            /// @todo Remove this stuff once implemented. We only want to log warnings once per element.
            bool warn = true;
            if (m_currentNodeName == nnBinormal)
            {
                if (warnBinormal)
                {
                    warnBinormal = false;
                }
                else
                {
                    warn = false;
                }
            }
            else if (m_currentNodeName == nnColorDiffuse)
            {
                if (warnColorDiffuse)
                {
                    warnColorDiffuse = false;
                }
                else
                {
                    warn = false;
                }
            }
            else if (m_currentNodeName == nnColorSpecular)
            {
                if (warnColorSpecular)
                {
                    warnColorSpecular = false;
                }
                else
                {
                    warn = false;
                }
            }
            if (warn) {
                ASSIMP_LOG_WARN_F("Vertex buffer attribute read not implemented for element: ", m_currentNodeName);
            }
        }

        // Advance
        NextNode();
    }

    // Sanity checks
    if (dest->positions.size() != dest->count) {
      throw DeadlyImportError(Formatter::format() << "Read only " << dest->positions.size() << " positions when should have read " << dest->count);
    }
    if (normals && dest->normals.size() != dest->count) {
        throw DeadlyImportError(Formatter::format() << "Read only " << dest->normals.size() << " normals when should have read " << dest->count);
    }
    if (tangents && dest->tangents.size() != dest->count) {
        throw DeadlyImportError(Formatter::format() << "Read only " << dest->tangents.size() << " tangents when should have read " << dest->count);
    }
    for(unsigned int i=0; i<dest->uvs.size(); ++i)
    {
        if (dest->uvs[i].size() != dest->count) {
            throw DeadlyImportError(Formatter::format() << "Read only " << dest->uvs[i].size()
                << " uvs for uv index " << i << " when should have read " << dest->count);
        }
    }
}

void OgreXmlSerializer::ReadSubMesh(MeshXml *mesh)
{
    static const char *anMaterial          = "material";
    static const char *anUseSharedVertices = "usesharedvertices";
    static const char *anCount             = "count";
    static const char *anV1                = "v1";
    static const char *anV2                = "v2";
    static const char *anV3                = "v3";
    static const char *anV4                = "v4";

    SubMeshXml* submesh = new SubMeshXml();

    if (HasAttribute(anMaterial)) {
        submesh->materialRef = ReadAttribute<std::string>(anMaterial);
    }
    if (HasAttribute(anUseSharedVertices)) {
        submesh->usesSharedVertexData = ReadAttribute<bool>(anUseSharedVertices);
    }

    ASSIMP_LOG_DEBUG_F( "Reading SubMesh ", mesh->subMeshes.size());
    ASSIMP_LOG_DEBUG_F( "  - Material: '", submesh->materialRef, "'");
    ASSIMP_LOG_DEBUG_F( "  - Uses shared geometry: ", (submesh->usesSharedVertexData ? "true" : "false"));

    // TODO: maybe we have always just 1 faces and 1 geometry and always in this order. this loop will only work correct, when the order
    // of faces and geometry changed, and not if we have more than one of one
    /// @todo Fix above comment with better read logic below

    bool quadWarned = false;

    NextNode();
    while(m_currentNodeName == nnFaces     ||
          m_currentNodeName == nnGeometry  ||
          m_currentNodeName == nnTextures  ||
          m_currentNodeName == nnBoneAssignments)
    {
        if (m_currentNodeName == nnFaces)
        {
            submesh->indexData->faceCount = ReadAttribute<uint32_t>(anCount);
            submesh->indexData->faces.reserve(submesh->indexData->faceCount);

            NextNode();
            while(m_currentNodeName == nnFace)
            {
                aiFace face;
                face.mNumIndices = 3;
                face.mIndices = new unsigned int[3];
                face.mIndices[0] = ReadAttribute<uint32_t>(anV1);
                face.mIndices[1] = ReadAttribute<uint32_t>(anV2);
                face.mIndices[2] = ReadAttribute<uint32_t>(anV3);

                /// @todo Support quads if Ogre even supports them in XML (I'm not sure but I doubt it)
                if (!quadWarned && HasAttribute(anV4)) {
                    ASSIMP_LOG_WARN("Submesh <face> has quads with <v4>, only triangles are supported at the moment!");
                    quadWarned = true;
                }

                submesh->indexData->faces.push_back(face);

                // Advance
                NextNode();
            }

            if (submesh->indexData->faces.size() == submesh->indexData->faceCount) {
                ASSIMP_LOG_DEBUG_F( "  - Faces ", submesh->indexData->faceCount);
            } else {
                throw DeadlyImportError(Formatter::format() << "Read only " << submesh->indexData->faces.size() << " faces when should have read " << submesh->indexData->faceCount);
            }
        } else if (m_currentNodeName == nnGeometry) {
            if (submesh->usesSharedVertexData) {
                throw DeadlyImportError("Found <geometry> in <submesh> when use shared geometry is true. Invalid mesh file.");
            }

            submesh->vertexData = new VertexDataXml();
            ReadGeometry(submesh->vertexData);
        } else if (m_currentNodeName == nnBoneAssignments) {
            ReadBoneAssignments(submesh->vertexData);
        }
        // Assimp incompatible/ignored nodes
        else {
            SkipCurrentNode();
        }
    }

    submesh->index = static_cast<unsigned int>(mesh->subMeshes.size());
    mesh->subMeshes.push_back(submesh);
}

void OgreXmlSerializer::ReadBoneAssignments(VertexDataXml *dest)
{
    if (!dest) {
        throw DeadlyImportError("Cannot read bone assignments, vertex data is null.");
    }

    static const char *anVertexIndex = "vertexindex";
    static const char *anBoneIndex   = "boneindex";
    static const char *anWeight      = "weight";

    std::set<uint32_t> influencedVertices;

    NextNode();
    while(m_currentNodeName == nnVertexBoneAssignment)
    {
        VertexBoneAssignment ba;
        ba.vertexIndex = ReadAttribute<uint32_t>(anVertexIndex);
        ba.boneIndex = ReadAttribute<uint16_t>(anBoneIndex);
        ba.weight = ReadAttribute<float>(anWeight);

        dest->boneAssignments.push_back(ba);
        influencedVertices.insert(ba.vertexIndex);

        NextNode();
    }

    /** Normalize bone weights.
        Some exporters won't care if the sum of all bone weights
        for a single vertex equals 1 or not, so validate here. */
    const float epsilon = 0.05f;
    for (const uint32_t vertexIndex : influencedVertices)
    {
        float sum = 0.0f;
        for (VertexBoneAssignmentList::const_iterator baIter=dest->boneAssignments.begin(), baEnd=dest->boneAssignments.end(); baIter != baEnd; ++baIter)
        {
            if (baIter->vertexIndex == vertexIndex)
                sum += baIter->weight;
        }
        if ((sum < (1.0f - epsilon)) || (sum > (1.0f + epsilon)))
        {
            for (auto &boneAssign : dest->boneAssignments)
            {
                if (boneAssign.vertexIndex == vertexIndex)
                    boneAssign.weight /= sum;
            }
        }
    }

    ASSIMP_LOG_DEBUG_F( "  - ", dest->boneAssignments.size(), " bone assignments");
}

// Skeleton

bool OgreXmlSerializer::ImportSkeleton(Assimp::IOSystem *pIOHandler, MeshXml *mesh)
{
    if (!mesh || mesh->skeletonRef.empty())
        return false;

    // Highly unusual to see in read world cases but support
    // XML mesh referencing a binary skeleton file.
    if (EndsWith(mesh->skeletonRef, ".skeleton", false))
    {
        if (OgreBinarySerializer::ImportSkeleton(pIOHandler, mesh))
            return true;

        /** Last fallback if .skeleton failed to be read. Try reading from
            .skeleton.xml even if the XML file referenced a binary skeleton.
            @note This logic was in the previous version and I don't want to break
            old code that might depends on it. */
        mesh->skeletonRef = mesh->skeletonRef + ".xml";
    }

    XmlReaderPtr reader = OpenReader(pIOHandler, mesh->skeletonRef);
    if (!reader.get())
        return false;

    Skeleton *skeleton = new Skeleton();
    OgreXmlSerializer serializer(reader.get());
    serializer.ReadSkeleton(skeleton);
    mesh->skeleton = skeleton;
    return true;
}

bool OgreXmlSerializer::ImportSkeleton(Assimp::IOSystem *pIOHandler, Mesh *mesh)
{
    if (!mesh || mesh->skeletonRef.empty())
        return false;

    XmlReaderPtr reader = OpenReader(pIOHandler, mesh->skeletonRef);
    if (!reader.get())
        return false;

    Skeleton *skeleton = new Skeleton();
    OgreXmlSerializer serializer(reader.get());
    serializer.ReadSkeleton(skeleton);
    mesh->skeleton = skeleton;
    return true;
}

XmlReaderPtr OgreXmlSerializer::OpenReader(Assimp::IOSystem *pIOHandler, const std::string &filename)
{
    if (!EndsWith(filename, ".skeleton.xml", false))
    {
        ASSIMP_LOG_ERROR_F("Imported Mesh is referencing to unsupported '", filename, "' skeleton file.");
        return XmlReaderPtr();
    }

    if (!pIOHandler->Exists(filename))
    {
        ASSIMP_LOG_ERROR_F("Failed to find skeleton file '", filename, "' that is referenced by imported Mesh.");
        return XmlReaderPtr();
    }

    std::unique_ptr<IOStream> file(pIOHandler->Open(filename));
    if (!file.get()) {
        throw DeadlyImportError("Failed to open skeleton file " + filename);
    }

    std::unique_ptr<CIrrXML_IOStreamReader> stream(new CIrrXML_IOStreamReader(file.get()));
    XmlReaderPtr reader = XmlReaderPtr(irr::io::createIrrXMLReader(stream.get()));
    if (!reader.get()) {
        throw DeadlyImportError("Failed to create XML reader for skeleton file " + filename);
    }
    return reader;
}

void OgreXmlSerializer::ReadSkeleton(Skeleton *skeleton)
{
    if (NextNode() != nnSkeleton) {
        throw DeadlyImportError("Root node is <" + m_currentNodeName + "> expecting <skeleton>");
    }

    ASSIMP_LOG_DEBUG("Reading Skeleton");

    // Optional blend mode from root node
    if (HasAttribute("blendmode")) {
        skeleton->blendMode = (ToLower(ReadAttribute<std::string>("blendmode")) == "cumulative"
            ? Skeleton::ANIMBLEND_CUMULATIVE : Skeleton::ANIMBLEND_AVERAGE);
    }

    NextNode();

    // Root level nodes
    while(m_currentNodeName == nnBones         ||
          m_currentNodeName == nnBoneHierarchy ||
          m_currentNodeName == nnAnimations    ||
          m_currentNodeName == nnAnimationLinks)
    {
        if (m_currentNodeName == nnBones)
            ReadBones(skeleton);
        else if (m_currentNodeName == nnBoneHierarchy)
            ReadBoneHierarchy(skeleton);
        else if (m_currentNodeName == nnAnimations)
            ReadAnimations(skeleton);
        else
            SkipCurrentNode();
    }
}

void OgreXmlSerializer::ReadAnimations(Skeleton *skeleton)
{
    if (skeleton->bones.empty()) {
        throw DeadlyImportError("Cannot read <animations> for a Skeleton without bones");
    }

    ASSIMP_LOG_DEBUG("  - Animations");

    NextNode();
    while(m_currentNodeName == nnAnimation)
    {
        Animation *anim = new Animation(skeleton);
        anim->name = ReadAttribute<std::string>("name");
        anim->length = ReadAttribute<float>("length");

        if (NextNode() != nnTracks) {
            throw DeadlyImportError(Formatter::format() << "No <tracks> found in <animation> " << anim->name);
        }

        ReadAnimationTracks(anim);
        skeleton->animations.push_back(anim);

        ASSIMP_LOG_DEBUG_F( "    ", anim->name, " (", anim->length, " sec, ", anim->tracks.size(), " tracks)");
    }
}

void OgreXmlSerializer::ReadAnimationTracks(Animation *dest)
{
    NextNode();
    while(m_currentNodeName == nnTrack)
    {
        VertexAnimationTrack track;
        track.type = VertexAnimationTrack::VAT_TRANSFORM;
        track.boneName = ReadAttribute<std::string>("bone");

        if (NextNode() != nnKeyFrames) {
            throw DeadlyImportError(Formatter::format() << "No <keyframes> found in <track> " << dest->name);
        }

        ReadAnimationKeyFrames(dest, &track);

        dest->tracks.push_back(track);
    }
}

void OgreXmlSerializer::ReadAnimationKeyFrames(Animation *anim, VertexAnimationTrack *dest)
{
    const aiVector3D zeroVec(0.f, 0.f, 0.f);

    NextNode();
    while(m_currentNodeName == nnKeyFrame)
    {
        TransformKeyFrame keyframe;
        keyframe.timePos = ReadAttribute<float>("time");

        NextNode();
        while(m_currentNodeName == nnTranslate || m_currentNodeName == nnRotate || m_currentNodeName == nnScale)
        {
            if (m_currentNodeName == nnTranslate)
            {
                keyframe.position.x = ReadAttribute<float>(anX);
                keyframe.position.y = ReadAttribute<float>(anY);
                keyframe.position.z = ReadAttribute<float>(anZ);
            }
            else if (m_currentNodeName == nnRotate)
            {
                float angle = ReadAttribute<float>("angle");

                if (NextNode() != nnAxis) {
                    throw DeadlyImportError("No axis specified for keyframe rotation in animation " + anim->name);
                }

                aiVector3D axis;
                axis.x = ReadAttribute<float>(anX);
                axis.y = ReadAttribute<float>(anY);
                axis.z = ReadAttribute<float>(anZ);
                if (axis.Equal(zeroVec))
                {
                    axis.x = 1.0f;
                    if (angle != 0) {
                        ASSIMP_LOG_WARN_F("Found invalid a key frame with a zero rotation axis in animation: ", anim->name);
                    }
                }
                keyframe.rotation = aiQuaternion(axis, angle);
            }
            else if (m_currentNodeName == nnScale)
            {
                keyframe.scale.x = ReadAttribute<float>(anX);
                keyframe.scale.y = ReadAttribute<float>(anY);
                keyframe.scale.z = ReadAttribute<float>(anZ);
            }

            NextNode();
        }

        dest->transformKeyFrames.push_back(keyframe);
    }
}

void OgreXmlSerializer::ReadBoneHierarchy(Skeleton *skeleton)
{
    if (skeleton->bones.empty()) {
        throw DeadlyImportError("Cannot read <bonehierarchy> for a Skeleton without bones");
    }

    while(NextNode() == nnBoneParent)
    {
        const std::string name = ReadAttribute<std::string>("bone");
        const std::string parentName = ReadAttribute<std::string>("parent");

        Bone *bone = skeleton->BoneByName(name);
        Bone *parent = skeleton->BoneByName(parentName);

        if (bone && parent)
            parent->AddChild(bone);
        else
            throw DeadlyImportError("Failed to find bones for parenting: Child " + name + " for parent " + parentName);
    }

    // Calculate bone matrices for root bones. Recursively calculates their children.
    for (size_t i=0, len=skeleton->bones.size(); i<len; ++i)
    {
        Bone *bone = skeleton->bones[i];
        if (!bone->IsParented())
            bone->CalculateWorldMatrixAndDefaultPose(skeleton);
    }
}

static bool BoneCompare(Bone *a, Bone *b)
{
    ai_assert( nullptr != a );
    ai_assert( nullptr != b );
    
    return (a->id < b->id);
}

void OgreXmlSerializer::ReadBones(Skeleton *skeleton)
{
    ASSIMP_LOG_DEBUG("  - Bones");

    NextNode();
    while(m_currentNodeName == nnBone)
    {
        Bone *bone = new Bone();
        bone->id = ReadAttribute<uint16_t>("id");
        bone->name = ReadAttribute<std::string>("name");

        NextNode();
        while(m_currentNodeName == nnPosition ||
              m_currentNodeName == nnRotation ||
              m_currentNodeName == nnScale)
        {
            if (m_currentNodeName == nnPosition)
            {
                bone->position.x = ReadAttribute<float>(anX);
                bone->position.y = ReadAttribute<float>(anY);
                bone->position.z = ReadAttribute<float>(anZ);
            }
            else if (m_currentNodeName == nnRotation)
            {
                float angle = ReadAttribute<float>("angle");

                if (NextNode() != nnAxis) {
                    throw DeadlyImportError(Formatter::format() << "No axis specified for bone rotation in bone " << bone->id);
                }

                aiVector3D axis;
                axis.x = ReadAttribute<float>(anX);
                axis.y = ReadAttribute<float>(anY);
                axis.z = ReadAttribute<float>(anZ);

                bone->rotation = aiQuaternion(axis, angle);
            }
            else if (m_currentNodeName == nnScale)
            {
                /// @todo Implement taking scale into account in matrix/pose calculations!
                if (HasAttribute("factor"))
                {
                    float factor = ReadAttribute<float>("factor");
                    bone->scale.Set(factor, factor, factor);
                }
                else
                {
                    if (HasAttribute(anX))
                        bone->scale.x = ReadAttribute<float>(anX);
                    if (HasAttribute(anY))
                        bone->scale.y = ReadAttribute<float>(anY);
                    if (HasAttribute(anZ))
                        bone->scale.z = ReadAttribute<float>(anZ);
                }
            }

            NextNode();
        }

        skeleton->bones.push_back(bone);
    }

    // Order bones by Id
    std::sort(skeleton->bones.begin(), skeleton->bones.end(), BoneCompare);

    // Validate that bone indexes are not skipped.
    /** @note Left this from original authors code, but not sure if this is strictly necessary
        as per the Ogre skeleton spec. It might be more that other (later) code in this imported does not break. */
    for (size_t i=0, len=skeleton->bones.size(); i<len; ++i)
    {
        Bone *b = skeleton->bones[i];
        ASSIMP_LOG_DEBUG_F( "    ", b->id, " ", b->name);

        if (b->id != static_cast<uint16_t>(i)) {
            throw DeadlyImportError(Formatter::format() << "Bone ids are not in sequence starting from 0. Missing index " << i);
        }
    }
}

} // Ogre
} // Assimp

#endif // ASSIMP_BUILD_NO_OGRE_IMPORTER
