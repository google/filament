/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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


/** @file  MD5Parser.h
 *  @brief Definition of the .MD5 parser class.
 *  http://www.modwiki.net/wiki/MD5_(file_format)
 */
#ifndef AI_MD5PARSER_H_INCLUDED
#define AI_MD5PARSER_H_INCLUDED

#include <assimp/types.h>
#include <assimp/ParsingUtils.h>
#include <vector>
#include <stdint.h>

struct aiFace;

namespace Assimp    {
namespace MD5           {

// ---------------------------------------------------------------------------
/** Represents a single element in a MD5 file
 *
 *  Elements are always contained in sections.
*/
struct Element
{
    //! Points to the starting point of the element
    //! Whitespace at the beginning and at the end have been removed,
    //! Elements are terminated with \0
    char* szStart;

    //! Original line number (can be used in error messages
    //! if a parsing error occurs)
    unsigned int iLineNumber;
};

typedef std::vector< Element > ElementList;

// ---------------------------------------------------------------------------
/** Represents a section of a MD5 file (such as the mesh or the joints section)
 *
 *  A section is always enclosed in { and } brackets.
*/
struct Section
{
    //! Original line number (can be used in error messages
    //! if a parsing error occurs)
    unsigned int iLineNumber;

    //! List of all elements which have been parsed in this section.
    ElementList mElements;

    //! Name of the section
    std::string mName;

    //! For global elements: the value of the element as string
    //! Iif !length() the section is not a global element
    std::string mGlobalValue;
};

typedef std::vector< Section> SectionList;

// ---------------------------------------------------------------------------
/** Basic information about a joint
*/
struct BaseJointDescription
{
    //! Name of the bone
    aiString mName;

    //! Parent index of the bone
    int mParentIndex;
};

// ---------------------------------------------------------------------------
/** Represents a bone (joint) descriptor in a MD5Mesh file
*/
struct BoneDesc : BaseJointDescription
{
    //! Absolute position of the bone
    aiVector3D mPositionXYZ;

    //! Absolute rotation of the bone
    aiVector3D mRotationQuat;
    aiQuaternion mRotationQuatConverted;

    //! Absolute transformation of the bone
    //! (temporary)
    aiMatrix4x4 mTransform;

    //! Inverse transformation of the bone
    //! (temporary)
    aiMatrix4x4 mInvTransform;

    //! Internal
    unsigned int mMap;
};

typedef std::vector< BoneDesc > BoneList;

// ---------------------------------------------------------------------------
/** Represents a bone (joint) descriptor in a MD5Anim file
*/
struct AnimBoneDesc : BaseJointDescription
{
    //! Flags (AI_MD5_ANIMATION_FLAG_xxx)
    unsigned int iFlags;

    //! Index of the first key that corresponds to this anim bone
    unsigned int iFirstKeyIndex;
};

typedef std::vector< AnimBoneDesc > AnimBoneList;


// ---------------------------------------------------------------------------
/** Represents a base frame descriptor in a MD5Anim file
*/
struct BaseFrameDesc
{
    aiVector3D vPositionXYZ;
    aiVector3D vRotationQuat;
};

typedef std::vector< BaseFrameDesc > BaseFrameList;

// ---------------------------------------------------------------------------
/** Represents a camera animation frame in a MDCamera file
*/
struct CameraAnimFrameDesc : BaseFrameDesc
{
    float fFOV;
};

typedef std::vector< CameraAnimFrameDesc > CameraFrameList;

// ---------------------------------------------------------------------------
/** Represents a frame descriptor in a MD5Anim file
*/
struct FrameDesc
{
    //! Index of the frame
    unsigned int iIndex;

    //! Animation keyframes - a large blob of data at first
    std::vector< float > mValues;
};

typedef std::vector< FrameDesc > FrameList;

// ---------------------------------------------------------------------------
/** Represents a vertex  descriptor in a MD5 file
*/
struct VertexDesc {
    VertexDesc() AI_NO_EXCEPT
    : mFirstWeight(0)
    , mNumWeights(0) {
        // empty
    }

    //! UV coordinate of the vertex
    aiVector2D mUV;

    //! Index of the first weight of the vertex in
    //! the vertex weight list
    unsigned int mFirstWeight;

    //! Number of weights assigned to this vertex
    unsigned int mNumWeights;
};

typedef std::vector< VertexDesc > VertexList;

// ---------------------------------------------------------------------------
/** Represents a vertex weight descriptor in a MD5 file
*/
struct WeightDesc
{
    //! Index of the bone to which this weight refers
    unsigned int mBone;

    //! The weight value
    float mWeight;

    //! The offset position of this weight
    // ! (in the coordinate system defined by the parent bone)
    aiVector3D vOffsetPosition;
};

typedef std::vector< WeightDesc > WeightList;
typedef std::vector< aiFace > FaceList;

// ---------------------------------------------------------------------------
/** Represents a mesh in a MD5 file
*/
struct MeshDesc
{
    //! Weights of the mesh
    WeightList mWeights;

    //! Vertices of the mesh
    VertexList mVertices;

    //! Faces of the mesh
    FaceList mFaces;

    //! Name of the shader (=texture) to be assigned to the mesh
    aiString mShader;
};

typedef std::vector< MeshDesc > MeshList;

// ---------------------------------------------------------------------------
// Convert a quaternion to its usual representation
inline void ConvertQuaternion (const aiVector3D& in, aiQuaternion& out) {

    out.x = in.x;
    out.y = in.y;
    out.z = in.z;

    const float t = 1.0f - (in.x*in.x) - (in.y*in.y) - (in.z*in.z);

    if (t < 0.0f)
        out.w = 0.0f;
    else out.w = std::sqrt (t);

    // Assimp convention.
    out.w *= -1.f;
}

// ---------------------------------------------------------------------------
/** Parses the data sections of a MD5 mesh file
*/
class MD5MeshParser
{
public:

    // -------------------------------------------------------------------
    /** Constructs a new MD5MeshParser instance from an existing
     *  preparsed list of file sections.
     *
     *  @param mSections List of file sections (output of MD5Parser)
     */
    explicit MD5MeshParser(SectionList& mSections);

    //! List of all meshes
    MeshList mMeshes;

    //! List of all joints
    BoneList mJoints;
};

// remove this flag if you need to the bounding box data
#define AI_MD5_PARSE_NO_BOUNDS

// ---------------------------------------------------------------------------
/** Parses the data sections of a MD5 animation file
*/
class MD5AnimParser
{
public:

    // -------------------------------------------------------------------
    /** Constructs a new MD5AnimParser instance from an existing
     *  preparsed list of file sections.
     *
     *  @param mSections List of file sections (output of MD5Parser)
     */
    explicit MD5AnimParser(SectionList& mSections);


    //! Output frame rate
    float fFrameRate;

    //! List of animation bones
    AnimBoneList mAnimatedBones;

    //! List of base frames
    BaseFrameList mBaseFrames;

    //! List of animation frames
    FrameList mFrames;

    //! Number of animated components
    unsigned int mNumAnimatedComponents;
};

// ---------------------------------------------------------------------------
/** Parses the data sections of a MD5 camera animation file
*/
class MD5CameraParser
{
public:

    // -------------------------------------------------------------------
    /** Constructs a new MD5CameraParser instance from an existing
     *  preparsed list of file sections.
     *
     *  @param mSections List of file sections (output of MD5Parser)
     */
    explicit MD5CameraParser(SectionList& mSections);


    //! Output frame rate
    float fFrameRate;

    //! List of cuts
    std::vector<unsigned int> cuts;

    //! Frames
    CameraFrameList frames;
};

// ---------------------------------------------------------------------------
/** Parses the block structure of MD5MESH and MD5ANIM files (but does no
 *  further processing)
*/
class MD5Parser
{
public:

    // -------------------------------------------------------------------
    /** Constructs a new MD5Parser instance from an existing buffer.
     *
     *  @param buffer File buffer
     *  @param fileSize Length of the file in bytes (excluding a terminal 0)
     */
    MD5Parser(char* buffer, unsigned int fileSize);


    // -------------------------------------------------------------------
    /** Report a specific error message and throw an exception
     *  @param error Error message to be reported
     *  @param line Index of the line where the error occurred
     */
    AI_WONT_RETURN static void ReportError (const char* error, unsigned int line) AI_WONT_RETURN_SUFFIX;

    // -------------------------------------------------------------------
    /** Report a specific warning
     *  @param warn Warn message to be reported
     *  @param line Index of the line where the error occurred
     */
    static void ReportWarning (const char* warn, unsigned int line);


    void ReportError (const char* error) {
        return ReportError(error, lineNumber);
    }

    void ReportWarning (const char* warn) {
        return ReportWarning(warn, lineNumber);
    }

public:

    //! List of all sections which have been read
    SectionList mSections;

private:

    // -------------------------------------------------------------------
    /** Parses a file section. The current file pointer must be outside
     *  of a section.
     *  @param out Receives the section data
     *  @return true if the end of the file has been reached
     *  @throws ImportErrorException if an error occurs
     */
    bool ParseSection(Section& out);

    // -------------------------------------------------------------------
    /** Parses the file header
     *  @throws ImportErrorException if an error occurs
     */
    void ParseHeader();


    // override these functions to make sure the line counter gets incremented
    // -------------------------------------------------------------------
    bool SkipLine( const char* in, const char** out)
    {
        ++lineNumber;
        return Assimp::SkipLine(in,out);
    }
    // -------------------------------------------------------------------
    bool SkipLine( )
    {
        return SkipLine(buffer,(const char**)&buffer);
    }
    // -------------------------------------------------------------------
    bool SkipSpacesAndLineEnd( const char* in, const char** out)
    {
        bool bHad = false;
        bool running = true;
        while (running) {
            if( *in == '\r' || *in == '\n') {
                 // we open files in binary mode, so there could be \r\n sequences ...
                if (!bHad)  {
                    bHad = true;
                    ++lineNumber;
                }
            }
            else if (*in == '\t' || *in == ' ')bHad = false;
            else break;
            in++;
        }
        *out = in;
        return *in != '\0';
    }
    // -------------------------------------------------------------------
    bool SkipSpacesAndLineEnd( )
    {
        return SkipSpacesAndLineEnd(buffer,(const char**)&buffer);
    }
    // -------------------------------------------------------------------
    bool SkipSpaces( )
    {
        return Assimp::SkipSpaces((const char**)&buffer);
    }

    char* buffer;
    unsigned int fileSize;
    unsigned int lineNumber;
};
}}

#endif // AI_MD5PARSER_H_INCLUDED
