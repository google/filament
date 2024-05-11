/** Defines the BHV motion capturing loader class */

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

/** @file BVHLoader.h
 *  @brief Biovision BVH import
 */

#ifndef AI_BVHLOADER_H_INC
#define AI_BVHLOADER_H_INC

#include <assimp/BaseImporter.h>

struct aiNode;

namespace Assimp
{

// --------------------------------------------------------------------------------
/** Loader class to read Motion Capturing data from a .bvh file.
 *
 * This format only contains a hierarchy of joints and a series of keyframes for
 * the hierarchy. It contains no actual mesh data, but we generate a dummy mesh
 * inside the loader just to be able to see something.
*/
class BVHLoader : public BaseImporter
{

    /** Possible animation channels for which the motion data holds the values */
    enum ChannelType
    {
        Channel_PositionX,
        Channel_PositionY,
        Channel_PositionZ,
        Channel_RotationX,
        Channel_RotationY,
        Channel_RotationZ
    };

    /** Collected list of node. Will be bones of the dummy mesh some day, addressed by their array index */
    struct Node
    {
        const aiNode* mNode;
        std::vector<ChannelType> mChannels;
        std::vector<float> mChannelValues; // motion data values for that node. Of size NumChannels * NumFrames

        Node()
        : mNode(nullptr)
        { }

        explicit Node( const aiNode* pNode) : mNode( pNode) { }
    };

public:

    BVHLoader();
    ~BVHLoader();

public:
    /** Returns whether the class can handle the format of the given file.
     * See BaseImporter::CanRead() for details. */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler, bool cs) const;

    void SetupProperties(const Importer* pImp);
    const aiImporterDesc* GetInfo () const;

protected:


    /** Imports the given file into the given scene structure.
     * See BaseImporter::InternReadFile() for details
     */
    void InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler);

protected:
    /** Reads the file */
    void ReadStructure( aiScene* pScene);

    /** Reads the hierarchy */
    void ReadHierarchy( aiScene* pScene);

    /** Reads a node and recursively its childs and returns the created node. */
    aiNode* ReadNode();

    /** Reads an end node and returns the created node. */
    aiNode* ReadEndSite( const std::string& pParentName);

    /** Reads a node offset for the given node */
    void ReadNodeOffset( aiNode* pNode);

    /** Reads the animation channels into the given node */
    void ReadNodeChannels( BVHLoader::Node& pNode);

    /** Reads the motion data */
    void ReadMotion( aiScene* pScene);

    /** Retrieves the next token */
    std::string GetNextToken();

    /** Reads the next token as a float */
    float GetNextTokenAsFloat();

    /** Aborts the file reading with an exception */
    AI_WONT_RETURN void ThrowException( const std::string& pError) AI_WONT_RETURN_SUFFIX;

    /** Constructs an animation for the motion data and stores it in the given scene */
    void CreateAnimation( aiScene* pScene);

protected:
    /** Filename, for a verbose error message */
    std::string mFileName;

    /** Buffer to hold the loaded file */
    std::vector<char> mBuffer;

    /** Next char to read from the buffer */
    std::vector<char>::const_iterator mReader;

    /** Current line, for error messages */
    unsigned int mLine;

    /** Collected list of nodes. Will be bones of the dummy mesh some day, addressed by their array index.
    * Also contain the motion data for the node's channels
    */
    std::vector<Node> mNodes;

    /** basic Animation parameters */
    float mAnimTickDuration;
    unsigned int mAnimNumFrames;

    bool noSkeletonMesh;
};

} // end of namespace Assimp

#endif // AI_BVHLOADER_H_INC
