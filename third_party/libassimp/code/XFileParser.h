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

/** @file Helper class to parse a XFile into a temporary structure */
#ifndef AI_XFILEPARSER_H_INC
#define AI_XFILEPARSER_H_INC

#include <string>
#include <vector>

#include <assimp/types.h>

namespace Assimp
{
    namespace XFile
    {
        struct Node;
        struct Mesh;
        struct Scene;
        struct Material;
        struct Animation;
        struct AnimBone;
    }

/** The XFileParser reads a XFile either in text or binary form and builds a temporary
 * data structure out of it.
 */
class XFileParser
{
public:
    /** Constructor. Creates a data structure out of the XFile given in the memory block.
     * @param pBuffer Null-terminated memory buffer containing the XFile
     */
    explicit XFileParser( const std::vector<char>& pBuffer);

    /** Destructor. Destroys all imported data along with it */
    ~XFileParser();

    /** Returns the temporary representation of the imported data */
    XFile::Scene* GetImportedData() const { return mScene; }

protected:
    void ParseFile();
    void ParseDataObjectTemplate();
    void ParseDataObjectFrame( XFile::Node *pParent);
    void ParseDataObjectTransformationMatrix( aiMatrix4x4& pMatrix);
    void ParseDataObjectMesh( XFile::Mesh* pMesh);
    void ParseDataObjectSkinWeights( XFile::Mesh* pMesh);
    void ParseDataObjectSkinMeshHeader( XFile::Mesh* pMesh);
    void ParseDataObjectMeshNormals( XFile::Mesh* pMesh);
    void ParseDataObjectMeshTextureCoords( XFile::Mesh* pMesh);
    void ParseDataObjectMeshVertexColors( XFile::Mesh* pMesh);
    void ParseDataObjectMeshMaterialList( XFile::Mesh* pMesh);
    void ParseDataObjectMaterial( XFile::Material* pMaterial);
    void ParseDataObjectAnimTicksPerSecond();
    void ParseDataObjectAnimationSet();
    void ParseDataObjectAnimation( XFile::Animation* pAnim);
    void ParseDataObjectAnimationKey( XFile::AnimBone *pAnimBone);
    void ParseDataObjectTextureFilename( std::string& pName);
    void ParseUnknownDataObject();

    //! places pointer to next begin of a token, and ignores comments
    void FindNextNoneWhiteSpace();

    //! returns next parseable token. Returns empty string if no token there
    std::string GetNextToken();

    //! reads header of dataobject including the opening brace.
    //! returns false if error happened, and writes name of object
    //! if there is one
    void readHeadOfDataObject( std::string* poName = NULL);

    //! checks for closing curly brace, throws exception if not there
    void CheckForClosingBrace();

    //! checks for one following semicolon, throws exception if not there
    void CheckForSemicolon();

    //! checks for a separator char, either a ',' or a ';'
    void CheckForSeparator();

  /// tests and possibly consumes a separator char, but does nothing if there was no separator
  void TestForSeparator();

    //! reads a x file style string
    void GetNextTokenAsString( std::string& poString);

    void ReadUntilEndOfLine();

    unsigned short ReadBinWord();
    unsigned int ReadBinDWord();
    unsigned int ReadInt();
    ai_real ReadFloat();
    aiVector2D ReadVector2();
    aiVector3D ReadVector3();
    aiColor3D ReadRGB();
    aiColor4D ReadRGBA();

    /** Throws an exception with a line number and the given text. */
    AI_WONT_RETURN void ThrowException( const std::string& pText) AI_WONT_RETURN_SUFFIX;

    /** Filters the imported hierarchy for some degenerated cases that some exporters produce.
     * @param pData The sub-hierarchy to filter
     */
    void FilterHierarchy( XFile::Node* pNode);

protected:
    unsigned int mMajorVersion, mMinorVersion; ///< version numbers
    bool mIsBinaryFormat; ///< true if the file is in binary, false if it's in text form
    unsigned int mBinaryFloatSize; ///< float size in bytes, either 4 or 8
    // counter for number arrays in binary format
    unsigned int mBinaryNumCount;

    const char* P;
    const char* End;

    /// Line number when reading in text format
    unsigned int mLineNumber;

    /// Imported data
    XFile::Scene* mScene;
};

}
#endif // AI_XFILEPARSER_H_INC
