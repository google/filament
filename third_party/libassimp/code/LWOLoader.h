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

/** @file Declaration of the LWO importer class. */
#ifndef AI_LWOLOADER_H_INCLUDED
#define AI_LWOLOADER_H_INCLUDED

#include <assimp/types.h>
#include <assimp/material.h>
#include <assimp/DefaultLogger.hpp>

#include "LWOFileData.h"
#include "BaseImporter.h"

#include <map>

struct aiTexture;
struct aiNode;
struct aiMaterial;

namespace Assimp    {
using namespace LWO;

// ---------------------------------------------------------------------------
/** Class to load LWO files.
 *
 *  @note  Methods named "xxxLWO2[xxx]" are used with the newer LWO2 format.
 *         Methods named "xxxLWOB[xxx]" are used with the older LWOB format.
 *         Methods named "xxxLWO[xxx]" are used with both formats.
 *         Methods named "xxx" are used to preprocess the loaded data -
 *         they aren't specific to one format version
*/
// ---------------------------------------------------------------------------
class LWOImporter : public BaseImporter
{
public:
    LWOImporter();
    ~LWOImporter();


public:

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
     * See BaseImporter::CanRead() for details.
     */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;


    // -------------------------------------------------------------------
    /** Called prior to ReadFile().
    * The function is a request to the importer to update its configuration
    * basing on the Importer's configuration property list.
    */
    void SetupProperties(const Importer* pImp);

protected:

    // -------------------------------------------------------------------
    // Get list of supported extensions
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
    * See BaseImporter::InternReadFile() for details
    */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

private:

    // -------------------------------------------------------------------
    /** Loads a LWO file in the older LWOB format (LW < 6)
     */
    void LoadLWOBFile();

    // -------------------------------------------------------------------
    /** Loads a LWO file in the newer LWO2 format (LW >= 6)
     */
    void LoadLWO2File();


    // -------------------------------------------------------------------
    /** Parsing functions used for all file format versions
    */
    inline void GetS0(std::string& out,unsigned int max);
    inline float GetF4();
    inline uint32_t GetU4();
    inline uint16_t GetU2();
    inline uint8_t  GetU1();


    // -------------------------------------------------------------------
    /** Loads a surface chunk from an LWOB file
     *  @param size Maximum size to be read, in bytes.
     */
    void LoadLWOBSurface(unsigned int size);

    // -------------------------------------------------------------------
    /** Loads a surface chunk from an LWO2 file
     *  @param size Maximum size to be read, in bytes.
     */
    void LoadLWO2Surface(unsigned int size);

    // -------------------------------------------------------------------
    /** Loads a texture block from a LWO2 file.
     *  @param size Maximum size to be read, in bytes.
     *  @param head Header of the SUF.BLOK header
     */
    void LoadLWO2TextureBlock(LE_NCONST IFF::SubChunkHeader* head,
        unsigned int size );

    // -------------------------------------------------------------------
    /** Loads a shader block from a LWO2 file.
     *  @param size Maximum size to be read, in bytes.
     *  @param head Header of the SUF.BLOK header
     */
    void LoadLWO2ShaderBlock(LE_NCONST IFF::SubChunkHeader* head,
        unsigned int size );

    // -------------------------------------------------------------------
    /** Loads an image map from a LWO2 file
     *  @param size Maximum size to be read, in bytes.
     *  @param tex Texture object to be filled
     */
    void LoadLWO2ImageMap(unsigned int size, LWO::Texture& tex );
    void LoadLWO2Gradient(unsigned int size, LWO::Texture& tex );
    void LoadLWO2Procedural(unsigned int size, LWO::Texture& tex );

    // loads the header - used by thethree functions above
    void LoadLWO2TextureHeader(unsigned int size, LWO::Texture& tex );

    // -------------------------------------------------------------------
    /** Loads the LWO tag list from the file
     *  @param size Maximum size to be read, in bytes.
     */
    void LoadLWOTags(unsigned int size);

    // -------------------------------------------------------------------
    /** Load polygons from a POLS chunk
     *  @param length Size of the chunk
    */
    void LoadLWO2Polygons(unsigned int length);
    void LoadLWOBPolygons(unsigned int length);

    // -------------------------------------------------------------------
    /** Load polygon tags from a PTAG chunk
     *  @param length Size of the chunk
    */
    void LoadLWO2PolygonTags(unsigned int length);

    // -------------------------------------------------------------------
    /** Load a vertex map from a VMAP/VMAD chunk
     *  @param length Size of the chunk
     *  @param perPoly Operate on per-polygon base?
    */
    void LoadLWO2VertexMap(unsigned int length, bool perPoly);

    // -------------------------------------------------------------------
    /** Load polygons from a PNTS chunk
     *  @param length Size of the chunk
    */
    void LoadLWOPoints(unsigned int length);

    // -------------------------------------------------------------------
    /** Load a clip from a CLIP chunk
     *  @param length Size of the chunk
    */
    void LoadLWO2Clip(unsigned int length);

    // -------------------------------------------------------------------
    /** Load an envelope from an EVL chunk
     *  @param length Size of the chunk
    */
    void LoadLWO2Envelope(unsigned int length);

    // -------------------------------------------------------------------
    /** Count vertices and faces in a LWOB/LWO2 file
    */
    void CountVertsAndFacesLWO2(unsigned int& verts,
        unsigned int& faces,
        uint16_t*& cursor,
        const uint16_t* const end,
        unsigned int max = UINT_MAX);

    void CountVertsAndFacesLWOB(unsigned int& verts,
        unsigned int& faces,
        LE_NCONST uint16_t*& cursor,
        const uint16_t* const end,
        unsigned int max = UINT_MAX);

    // -------------------------------------------------------------------
    /** Read vertices and faces in a LWOB/LWO2 file
    */
    void CopyFaceIndicesLWO2(LWO::FaceList::iterator& it,
        uint16_t*& cursor,
        const uint16_t* const end);

    // -------------------------------------------------------------------
    void CopyFaceIndicesLWOB(LWO::FaceList::iterator& it,
        LE_NCONST uint16_t*& cursor,
        const uint16_t* const end,
        unsigned int max = UINT_MAX);

    // -------------------------------------------------------------------
    /** Resolve the tag and surface lists that have been loaded.
    *   Generates the mMapping table.
    */
    void ResolveTags();

    // -------------------------------------------------------------------
    /** Resolve the clip list that has been loaded.
    *   Replaces clip references with real clips.
    */
    void ResolveClips();

    // -------------------------------------------------------------------
    /** Add a texture list to an output material description.
     *
     *  @param pcMat Output material
     *  @param in Input texture list
     *  @param type Type identifier of the texture list
    */
    bool HandleTextures(aiMaterial* pcMat, const TextureList& in,
        aiTextureType type);

    // -------------------------------------------------------------------
    /** Adjust a texture path
    */
    void AdjustTexturePath(std::string& out);

    // -------------------------------------------------------------------
    /** Convert a LWO surface description to an ASSIMP material
    */
    void ConvertMaterial(const LWO::Surface& surf,aiMaterial* pcMat);


    // -------------------------------------------------------------------
    /** Get a list of all UV/VC channels required by a specific surface.
     *
     *  @param surf Working surface
     *  @param layer Working layer
     *  @param out Output list. The members are indices into the
     *    UV/VC channel lists of the layer
    */
    void FindUVChannels(/*const*/ LWO::Surface& surf,
        LWO::SortedRep& sorted,
        /*const*/ LWO::Layer& layer,
        unsigned int out[AI_MAX_NUMBER_OF_TEXTURECOORDS]);

    // -------------------------------------------------------------------
    char FindUVChannels(LWO::TextureList& list,
        LWO::Layer& layer,LWO::UVChannel& uv, unsigned int next);

    // -------------------------------------------------------------------
    void FindVCChannels(const LWO::Surface& surf,
        LWO::SortedRep& sorted,
        const LWO::Layer& layer,
        unsigned int out[AI_MAX_NUMBER_OF_COLOR_SETS]);

    // -------------------------------------------------------------------
    /** Generate the final node graph
     *  Unused nodes are deleted.
     *  @param apcNodes Flat list of nodes
    */
    void GenerateNodeGraph(std::map<uint16_t,aiNode*>& apcNodes);

    // -------------------------------------------------------------------
    /** Add children to a node
     *  @param node Node to become a father
     *  @param parent Index of the node
     *  @param apcNodes Flat list of nodes - used nodes are set to NULL.
    */
    void AddChildren(aiNode* node, uint16_t parent,
        std::vector<aiNode*>& apcNodes);

    // -------------------------------------------------------------------
    /** Read a variable sized integer
     *  @param inout Input and output buffer
    */
    int ReadVSizedIntLWO2(uint8_t*& inout);

    // -------------------------------------------------------------------
    /** Assign a value from a VMAP to a vertex and all vertices
     *  attached to it.
     *  @param base VMAP destination data
     *  @param numRead Number of float's to be read
     *  @param idx Absolute index of the first vertex
     *  @param data Value of the VMAP to be assigned - read numRead
     *    floats from this array.
    */
    void DoRecursiveVMAPAssignment(VMapEntry* base, unsigned int numRead,
        unsigned int idx, float* data);

    // -------------------------------------------------------------------
    /** Compute normal vectors for a mesh
     *  @param mesh Input mesh
     *  @param smoothingGroups Smoothing-groups-per-face array
     *  @param surface Surface for the mesh
    */
    void ComputeNormals(aiMesh* mesh, const std::vector<unsigned int>& smoothingGroups,
        const LWO::Surface& surface);


    // -------------------------------------------------------------------
    /** Setup a new texture after the corresponding chunk was
     *  encountered in the file.
     *  @param list Texture list
     *  @param size Maximum number of bytes to be read
     *  @return Pointer to new texture
    */
    LWO::Texture* SetupNewTextureLWOB(LWO::TextureList& list,
        unsigned int size);

protected:

    /** true if the file is a LWO2 file*/
    bool mIsLWO2;

    /** true if the file is a LXOB file*/
    bool mIsLXOB;

    /** Temporary list of layers from the file */
    LayerList* mLayers;

    /** Pointer to the current layer */
    LWO::Layer* mCurLayer;

    /** Temporary tag list from the file */
    TagList* mTags;

    /** Mapping table to convert from tag to surface indices.
        UINT_MAX indicates that a no corresponding surface is available */
    TagMappingTable* mMapping;

    /** Temporary surface list from the file */
    SurfaceList* mSurfaces;

    /** Temporary clip list from the file */
    ClipList mClips;

    /** Temporary envelope list from the file */
    EnvelopeList mEnvelopes;

    /** file buffer */
    uint8_t* mFileBuffer;

    /** Size of the file, in bytes */
    unsigned int fileSize;

    /** Output scene */
    aiScene* pScene;

    /** Configuration option: speed flag set? */
    bool configSpeedFlag;

    /** Configuration option: index of layer to be loaded */
    unsigned int configLayerIndex;

    /** Configuration option: name of layer to be loaded */
    std::string  configLayerName;

    /** True if we have a named layer */
    bool hasNamedLayer;
};


// ------------------------------------------------------------------------------------------------
inline float LWOImporter::GetF4()
{
    float f;
    ::memcpy(&f, mFileBuffer, 4);
    mFileBuffer += 4;
    AI_LSWAP4(f);
    return f;
}

// ------------------------------------------------------------------------------------------------
inline uint32_t LWOImporter::GetU4()
{
    uint32_t f;
    ::memcpy(&f, mFileBuffer, 4);
    mFileBuffer += 4;
    AI_LSWAP4(f);
    return f;
}

// ------------------------------------------------------------------------------------------------
inline uint16_t LWOImporter::GetU2()
{
    uint16_t f;
    ::memcpy(&f, mFileBuffer, 2);
    mFileBuffer += 2;
    AI_LSWAP2(f);
    return f;
}

// ------------------------------------------------------------------------------------------------
inline uint8_t LWOImporter::GetU1()
{
    return *mFileBuffer++;
}

// ------------------------------------------------------------------------------------------------
inline int LWOImporter::ReadVSizedIntLWO2(uint8_t*& inout)
{
    int i;
    int c = *inout;inout++;
    if(c != 0xFF)
    {
        i = c << 8;
        c = *inout;inout++;
        i |= c;
    }
    else
    {
        c = *inout;inout++;
        i = c << 16;
        c = *inout;inout++;
        i |= c << 8;
        c = *inout;inout++;
        i |= c;
    }
    return i;
}

// ------------------------------------------------------------------------------------------------
inline void LWOImporter::GetS0(std::string& out,unsigned int max)
{
    unsigned int iCursor = 0;
    const char*sz = (const char*)mFileBuffer;
    while (*mFileBuffer)
    {
        if (++iCursor > max)
        {
            DefaultLogger::get()->warn("LWO: Invalid file, string is is too long");
            break;
        }
        ++mFileBuffer;
    }
    size_t len = (size_t) ((const char*)mFileBuffer-sz);
    out = std::string(sz,len);
    mFileBuffer += (len&0x1 ? 1 : 2);
}



} // end of namespace Assimp

#endif // AI_LWOIMPORTER_H_INCLUDED
