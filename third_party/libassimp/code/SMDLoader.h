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

/** @file  SMDLoader.h
 *  @brief Definition of the Valve SMD file format
 */

#ifndef AI_SMDLOADER_H_INCLUDED
#define AI_SMDLOADER_H_INCLUDED

// internal headers
#include "BaseImporter.h"
#include "ParsingUtils.h"

// public Assimp headers
#include <assimp/types.h>
#include <assimp/texture.h>
#include <assimp/anim.h>
#include <assimp/material.h>

struct aiNode;

// STL headers
#include <vector>

namespace Assimp    {

namespace SMD   {

// ---------------------------------------------------------------------------
/** Data structure for a vertex in a SMD file
*/
struct Vertex
{
    Vertex() : iParentNode(UINT_MAX)
     {}

    //! Vertex position, normal and texture coordinate
    aiVector3D pos,nor,uv;

    //! Vertex parent node
    unsigned int iParentNode;

    //! Links to bones: pair.first is the bone index,
    //! pair.second is the vertex weight.
    //! WARN: The remaining weight (to reach 1.0f) is assigned
    //! to the parent node/bone
    std::vector<std::pair<unsigned int, float> > aiBoneLinks;
};

// ---------------------------------------------------------------------------
/** Data structure for a face in a SMD file
*/
struct Face
{
    Face() : iTexture(0x0)
     {}

    //! Texture index for the face
    unsigned int iTexture;

    //! The three vertices of the face
    Vertex avVertices[3];
};

// ---------------------------------------------------------------------------
/** Data structure for a bone in a SMD file
*/
struct Bone
{
    //! Default constructor
    Bone() : iParent(UINT_MAX), bIsUsed(false)
    {
    }

    //! Destructor
    ~Bone()
    {
    }

    //! Name of the bone
    std::string mName;

    //! Parent of the bone
    uint32_t iParent;

    //! Animation of the bone
    struct Animation
    {
        //! Public default constructor
        Animation()
            : iFirstTimeKey()
        {
            asKeys.reserve(20);
        }

        //! Data structure for a matrix key
        struct MatrixKey
        {
            //! Matrix at this time
            aiMatrix4x4 matrix;

            //! Absolute transformation matrix
            aiMatrix4x4 matrixAbsolute;

            //! Position
            aiVector3D vPos;

            //! Rotation (euler angles)
            aiVector3D vRot;

            //! Current time. may be negative, this
            //! will be fixed later
            double dTime;
        };

        //! Index of the key with the smallest time value
        uint32_t iFirstTimeKey;

        //! Array of matrix keys
        std::vector<MatrixKey> asKeys;

    } sAnim;

    //! Offset matrix of the bone
    aiMatrix4x4 mOffsetMatrix;

    //! true if the bone is referenced by at least one mesh
    bool bIsUsed;
};

} //! namespace SMD

// ---------------------------------------------------------------------------
/** Used to load Half-life 1 and 2 SMD models
*/
class ASSIMP_API SMDImporter : public BaseImporter
{
public:
    SMDImporter();
    ~SMDImporter();


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
    /** Return importer meta information.
     * See #BaseImporter::GetInfo for the details
     */
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
    * See BaseImporter::InternReadFile() for details
    */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

protected:

    // -------------------------------------------------------------------
    /** Parse the SMD file and create the output scene
    */
    void ParseFile();

    // -------------------------------------------------------------------
    /** Parse the triangles section of the SMD file
     * \param szCurrent Current position in the file. Points to the first
     * data line of the section.
     * \param szCurrentOut Receives a pointer to the heading line of
     * the next section (or to EOF)
    */
    void ParseTrianglesSection(const char* szCurrent,
        const char** szCurrentOut);

    // -------------------------------------------------------------------
    /** Parse the vertex animation section in VTA files
     * \param szCurrent Current position in the file. Points to the first
     * data line of the section.
     * \param szCurrentOut Receives a pointer to the heading line of
     * the next section (or to EOF)
    */
    void ParseVASection(const char* szCurrent,
        const char** szCurrentOut);

    // -------------------------------------------------------------------
    /** Parse the nodes section of the SMD file
     * \param szCurrent Current position in the file. Points to the first
     * data line of the section.
     * \param szCurrentOut Receives a pointer to the heading line of
     * the next section (or to EOF)
    */
    void ParseNodesSection(const char* szCurrent,
        const char** szCurrentOut);

    // -------------------------------------------------------------------
    /** Parse the skeleton section of the SMD file
     * \param szCurrent Current position in the file. Points to the first
     * data line of the section.
     * \param szCurrentOut Receives a pointer to the heading line of
     * the next section (or to EOF)
    */
    void ParseSkeletonSection(const char* szCurrent,
        const char** szCurrentOut);

    // -------------------------------------------------------------------
    /** Parse a single triangle in the SMD file
     * \param szCurrent Current position in the file. Points to the first
     * data line of the section.
     * \param szCurrentOut Receives the output cursor position
    */
    void ParseTriangle(const char* szCurrent,
        const char** szCurrentOut);


    // -------------------------------------------------------------------
    /** Parse a single vertex in the SMD file
     * \param szCurrent Current position in the file. Points to the first
     * data line of the section.
     * \param szCurrentOut Receives the output cursor position
     * \param vertex Vertex to be filled
    */
    void ParseVertex(const char* szCurrent,
        const char** szCurrentOut, SMD::Vertex& vertex,
        bool bVASection = false);

    // -------------------------------------------------------------------
    /** Get  the index of a texture. If the texture was not yet known
     *  it will be added to the internal texture list.
     * \param filename Name of the texture
     * \return Value texture index
     */
    unsigned int GetTextureIndex(const std::string& filename);

    // -------------------------------------------------------------------
    /** Computes absolute bone transformations
     * All output transformations are in worldspace.
     */
    void ComputeAbsoluteBoneTransformations();


    // -------------------------------------------------------------------
    /** Parse a line in the skeleton section
     */
    void ParseSkeletonElement(const char* szCurrent,
        const char** szCurrentOut,int iTime);

    // -------------------------------------------------------------------
    /** Parse a line in the nodes section
     */
    void ParseNodeInfo(const char* szCurrent,
        const char** szCurrentOut);


    // -------------------------------------------------------------------
    /** Parse a floating-point value
     */
    bool ParseFloat(const char* szCurrent,
        const char** szCurrentOut, float& out);

    // -------------------------------------------------------------------
    /** Parse an unsigned integer. There may be no sign!
     */
    bool ParseUnsignedInt(const char* szCurrent,
        const char** szCurrentOut, unsigned int& out);

    // -------------------------------------------------------------------
    /** Parse a signed integer. Signs (+,-) are handled.
     */
    bool ParseSignedInt(const char* szCurrent,
        const char** szCurrentOut, int& out);

    // -------------------------------------------------------------------
    /** Fix invalid time values in the file
     */
    void FixTimeValues();

    // -------------------------------------------------------------------
    /** Add all children of a bone as subnodes to a node
     * \param pcNode Parent node
     * \param iParent Parent bone index
     */
    void AddBoneChildren(aiNode* pcNode, uint32_t iParent);

    // -------------------------------------------------------------------
    /** Build output meshes/materials/nodes/animations
     */
    void CreateOutputMeshes();
    void CreateOutputNodes();
    void CreateOutputAnimations();
    void CreateOutputMaterials();


    // -------------------------------------------------------------------
    /** Print a log message together with the current line number
     */
    void LogErrorNoThrow(const char* msg);
    void LogWarning(const char* msg);


    // -------------------------------------------------------------------
    inline bool SkipLine( const char* in, const char** out)
    {
        Assimp::SkipLine(in,out);
        ++iLineNumber;
        return true;
    }
    // -------------------------------------------------------------------
    inline bool SkipSpacesAndLineEnd( const char* in, const char** out)
    {
        ++iLineNumber;
        return Assimp::SkipSpacesAndLineEnd(in,out);
    }

private:

    /** Configuration option: frame to be loaded */
    unsigned int configFrameID;

    /** Buffer to hold the loaded file */
    std::vector<char> mBuffer;

    /** Output scene to be filled
    */
    aiScene* pScene;

    /** Size of the input file in bytes
     */
    unsigned int iFileSize;

    /** Array of textures found in the file
     */
    std::vector<std::string> aszTextures;

    /** Array of triangles found in the file
     */
    std::vector<SMD::Face> asTriangles;

    /** Array of bones found in the file
     */
    std::vector<SMD::Bone> asBones;

    /** Smallest frame index found in the skeleton
     */
    int iSmallestFrame;

    /** Length of the whole animation, in frames
     */
    double dLengthOfAnim;

    /** Do we have texture coordinates?
     */
    bool bHasUVs;

    /** Current line numer
     */
    unsigned int iLineNumber;

};

} // end of namespace Assimp

#endif // AI_SMDIMPORTER_H_INC
