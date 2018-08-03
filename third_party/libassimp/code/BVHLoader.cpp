/** Implementation of the BVH loader */
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


#ifndef ASSIMP_BUILD_NO_BVH_IMPORTER

#include "BVHLoader.h"
#include "fast_atof.h"
#include "SkeletonMeshBuilder.h"
#include <assimp/Importer.hpp>
#include <memory>
#include "TinyFormatter.h"
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

using namespace Assimp;
using namespace Assimp::Formatter;

static const aiImporterDesc desc = {
    "BVH Importer (MoCap)",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "bvh"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
BVHLoader::BVHLoader()
    : mLine(),
    mAnimTickDuration(),
    mAnimNumFrames(),
    noSkeletonMesh()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
BVHLoader::~BVHLoader()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool BVHLoader::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool cs) const
{
    // check file extension
    const std::string extension = GetExtension(pFile);

    if( extension == "bvh")
        return true;

    if ((!extension.length() || cs) && pIOHandler) {
        const char* tokens[] = {"HIERARCHY"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void BVHLoader::SetupProperties(const Importer* pImp)
{
    noSkeletonMesh = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_NO_SKELETON_MESHES,0) != 0;
}

// ------------------------------------------------------------------------------------------------
// Loader meta information
const aiImporterDesc* BVHLoader::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void BVHLoader::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
    mFileName = pFile;

    // read file into memory
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open file " + pFile + ".");

    size_t fileSize = file->FileSize();
    if( fileSize == 0)
        throw DeadlyImportError( "File is too small.");

    mBuffer.resize( fileSize);
    file->Read( &mBuffer.front(), 1, fileSize);

    // start reading
    mReader = mBuffer.begin();
    mLine = 1;
    ReadStructure( pScene);

    if (!noSkeletonMesh) {
        // build a dummy mesh for the skeleton so that we see something at least
        SkeletonMeshBuilder meshBuilder( pScene);
    }

    // construct an animation from all the motion data we read
    CreateAnimation( pScene);
}

// ------------------------------------------------------------------------------------------------
// Reads the file
void BVHLoader::ReadStructure( aiScene* pScene)
{
    // first comes hierarchy
    std::string header = GetNextToken();
    if( header != "HIERARCHY")
        ThrowException( "Expected header string \"HIERARCHY\".");
    ReadHierarchy( pScene);

    // then comes the motion data
    std::string motion = GetNextToken();
    if( motion != "MOTION")
        ThrowException( "Expected beginning of motion data \"MOTION\".");
    ReadMotion( pScene);
}

// ------------------------------------------------------------------------------------------------
// Reads the hierarchy
void BVHLoader::ReadHierarchy( aiScene* pScene)
{
    std::string root = GetNextToken();
    if( root != "ROOT")
        ThrowException( "Expected root node \"ROOT\".");

    // Go read the hierarchy from here
    pScene->mRootNode = ReadNode();
}

// ------------------------------------------------------------------------------------------------
// Reads a node and recursively its childs and returns the created node;
aiNode* BVHLoader::ReadNode()
{
    // first token is name
    std::string nodeName = GetNextToken();
    if( nodeName.empty() || nodeName == "{")
        ThrowException( format() << "Expected node name, but found \"" << nodeName << "\"." );

    // then an opening brace should follow
    std::string openBrace = GetNextToken();
    if( openBrace != "{")
        ThrowException( format() << "Expected opening brace \"{\", but found \"" << openBrace << "\"." );

    // Create a node
    aiNode* node = new aiNode( nodeName);
    std::vector<aiNode*> childNodes;

    // and create an bone entry for it
    mNodes.push_back( Node( node));
    Node& internNode = mNodes.back();

    // now read the node's contents
    while( 1)
    {
        std::string token = GetNextToken();

        // node offset to parent node
        if( token == "OFFSET")
            ReadNodeOffset( node);
        else if( token == "CHANNELS")
            ReadNodeChannels( internNode);
        else if( token == "JOINT")
        {
            // child node follows
            aiNode* child = ReadNode();
            child->mParent = node;
            childNodes.push_back( child);
        }
        else if( token == "End")
        {
            // The real symbol is "End Site". Second part comes in a separate token
            std::string siteToken = GetNextToken();
            if( siteToken != "Site")
                ThrowException( format() << "Expected \"End Site\" keyword, but found \"" << token << " " << siteToken << "\"." );

            aiNode* child = ReadEndSite( nodeName);
            child->mParent = node;
            childNodes.push_back( child);
        }
        else if( token == "}")
        {
            // we're done with that part of the hierarchy
            break;
        } else
        {
            // everything else is a parse error
            ThrowException( format() << "Unknown keyword \"" << token << "\"." );
        }
    }

    // add the child nodes if there are any
    if( childNodes.size() > 0)
    {
        node->mNumChildren = static_cast<unsigned int>(childNodes.size());
        node->mChildren = new aiNode*[node->mNumChildren];
        std::copy( childNodes.begin(), childNodes.end(), node->mChildren);
    }

    // and return the sub-hierarchy we built here
    return node;
}

// ------------------------------------------------------------------------------------------------
// Reads an end node and returns the created node.
aiNode* BVHLoader::ReadEndSite( const std::string& pParentName)
{
    // check opening brace
    std::string openBrace = GetNextToken();
    if( openBrace != "{")
        ThrowException( format() << "Expected opening brace \"{\", but found \"" << openBrace << "\".");

    // Create a node
    aiNode* node = new aiNode( "EndSite_" + pParentName);

    // now read the node's contents. Only possible entry is "OFFSET"
    while( 1)
    {
        std::string token = GetNextToken();

        // end node's offset
        if( token == "OFFSET")
        {
            ReadNodeOffset( node);
        }
        else if( token == "}")
        {
            // we're done with the end node
            break;
        } else
        {
            // everything else is a parse error
            ThrowException( format() << "Unknown keyword \"" << token << "\"." );
        }
    }

    // and return the sub-hierarchy we built here
    return node;
}
// ------------------------------------------------------------------------------------------------
// Reads a node offset for the given node
void BVHLoader::ReadNodeOffset( aiNode* pNode)
{
    // Offset consists of three floats to read
    aiVector3D offset;
    offset.x = GetNextTokenAsFloat();
    offset.y = GetNextTokenAsFloat();
    offset.z = GetNextTokenAsFloat();

    // build a transformation matrix from it
    pNode->mTransformation = aiMatrix4x4( 1.0f, 0.0f, 0.0f, offset.x, 0.0f, 1.0f, 0.0f, offset.y,
        0.0f, 0.0f, 1.0f, offset.z, 0.0f, 0.0f, 0.0f, 1.0f);
}

// ------------------------------------------------------------------------------------------------
// Reads the animation channels for the given node
void BVHLoader::ReadNodeChannels( BVHLoader::Node& pNode)
{
    // number of channels. Use the float reader because we're lazy
    float numChannelsFloat = GetNextTokenAsFloat();
    unsigned int numChannels = (unsigned int) numChannelsFloat;

    for( unsigned int a = 0; a < numChannels; a++)
    {
        std::string channelToken = GetNextToken();

        if( channelToken == "Xposition")
            pNode.mChannels.push_back( Channel_PositionX);
        else if( channelToken == "Yposition")
            pNode.mChannels.push_back( Channel_PositionY);
        else if( channelToken == "Zposition")
            pNode.mChannels.push_back( Channel_PositionZ);
        else if( channelToken == "Xrotation")
            pNode.mChannels.push_back( Channel_RotationX);
        else if( channelToken == "Yrotation")
            pNode.mChannels.push_back( Channel_RotationY);
        else if( channelToken == "Zrotation")
            pNode.mChannels.push_back( Channel_RotationZ);
        else
            ThrowException( format() << "Invalid channel specifier \"" << channelToken << "\"." );
    }
}

// ------------------------------------------------------------------------------------------------
// Reads the motion data
void BVHLoader::ReadMotion( aiScene* /*pScene*/)
{
    // Read number of frames
    std::string tokenFrames = GetNextToken();
    if( tokenFrames != "Frames:")
        ThrowException( format() << "Expected frame count \"Frames:\", but found \"" << tokenFrames << "\".");

    float numFramesFloat = GetNextTokenAsFloat();
    mAnimNumFrames = (unsigned int) numFramesFloat;

    // Read frame duration
    std::string tokenDuration1 = GetNextToken();
    std::string tokenDuration2 = GetNextToken();
    if( tokenDuration1 != "Frame" || tokenDuration2 != "Time:")
        ThrowException( format() << "Expected frame duration \"Frame Time:\", but found \"" << tokenDuration1 << " " << tokenDuration2 << "\"." );

    mAnimTickDuration = GetNextTokenAsFloat();

    // resize value vectors for each node
    for( std::vector<Node>::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
        it->mChannelValues.reserve( it->mChannels.size() * mAnimNumFrames);

    // now read all the data and store it in the corresponding node's value vector
    for( unsigned int frame = 0; frame < mAnimNumFrames; ++frame)
    {
        // on each line read the values for all nodes
        for( std::vector<Node>::iterator it = mNodes.begin(); it != mNodes.end(); ++it)
        {
            // get as many values as the node has channels
            for( unsigned int c = 0; c < it->mChannels.size(); ++c)
                it->mChannelValues.push_back( GetNextTokenAsFloat());
        }

        // after one frame worth of values for all nodes there should be a newline, but we better don't rely on it
    }
}

// ------------------------------------------------------------------------------------------------
// Retrieves the next token
std::string BVHLoader::GetNextToken()
{
    // skip any preceding whitespace
    while( mReader != mBuffer.end())
    {
        if( !isspace( *mReader))
            break;

        // count lines
        if( *mReader == '\n')
            mLine++;

        ++mReader;
    }

    // collect all chars till the next whitespace. BVH is easy in respect to that.
    std::string token;
    while( mReader != mBuffer.end())
    {
        if( isspace( *mReader))
            break;

        token.push_back( *mReader);
        ++mReader;

        // little extra logic to make sure braces are counted correctly
        if( token == "{" || token == "}")
            break;
    }

    // empty token means end of file, which is just fine
    return token;
}

// ------------------------------------------------------------------------------------------------
// Reads the next token as a float
float BVHLoader::GetNextTokenAsFloat()
{
    std::string token = GetNextToken();
    if( token.empty())
        ThrowException( "Unexpected end of file while trying to read a float");

    // check if the float is valid by testing if the atof() function consumed every char of the token
    const char* ctoken = token.c_str();
    float result = 0.0f;
    ctoken = fast_atoreal_move<float>( ctoken, result);

    if( ctoken != token.c_str() + token.length())
        ThrowException( format() << "Expected a floating point number, but found \"" << token << "\"." );

    return result;
}

// ------------------------------------------------------------------------------------------------
// Aborts the file reading with an exception
AI_WONT_RETURN void BVHLoader::ThrowException( const std::string& pError)
{
    throw DeadlyImportError( format() << mFileName << ":" << mLine << " - " << pError);
}

// ------------------------------------------------------------------------------------------------
// Constructs an animation for the motion data and stores it in the given scene
void BVHLoader::CreateAnimation( aiScene* pScene)
{
    // create the animation
    pScene->mNumAnimations = 1;
    pScene->mAnimations = new aiAnimation*[1];
    aiAnimation* anim = new aiAnimation;
    pScene->mAnimations[0] = anim;

    // put down the basic parameters
    anim->mName.Set( "Motion");
    anim->mTicksPerSecond = 1.0 / double( mAnimTickDuration);
    anim->mDuration = double( mAnimNumFrames - 1);

    // now generate the tracks for all nodes
    anim->mNumChannels = static_cast<unsigned int>(mNodes.size());
    anim->mChannels = new aiNodeAnim*[anim->mNumChannels];

    // FIX: set the array elements to NULL to ensure proper deletion if an exception is thrown
    for (unsigned int i = 0; i < anim->mNumChannels;++i)
        anim->mChannels[i] = NULL;

    for( unsigned int a = 0; a < anim->mNumChannels; a++)
    {
        const Node& node = mNodes[a];
        const std::string nodeName = std::string( node.mNode->mName.data );
        aiNodeAnim* nodeAnim = new aiNodeAnim;
        anim->mChannels[a] = nodeAnim;
        nodeAnim->mNodeName.Set( nodeName);

        // translational part, if given
        if( node.mChannels.size() == 6)
        {
            nodeAnim->mNumPositionKeys = mAnimNumFrames;
            nodeAnim->mPositionKeys = new aiVectorKey[mAnimNumFrames];
            aiVectorKey* poskey = nodeAnim->mPositionKeys;
            for( unsigned int fr = 0; fr < mAnimNumFrames; ++fr)
            {
                poskey->mTime = double( fr);

                // Now compute all translations in the right order
                for( unsigned int channel = 0; channel < 3; ++channel)
                {
                    switch( node.mChannels[channel])
                    {
                    case Channel_PositionX: poskey->mValue.x = node.mChannelValues[fr * node.mChannels.size() + channel]; break;
                    case Channel_PositionY: poskey->mValue.y = node.mChannelValues[fr * node.mChannels.size() + channel]; break;
                    case Channel_PositionZ: poskey->mValue.z = node.mChannelValues[fr * node.mChannels.size() + channel]; break;
                    default: throw DeadlyImportError( "Unexpected animation channel setup at node " + nodeName );
                    }
                }
                ++poskey;
            }
        } else
        {
            // if no translation part is given, put a default sequence
            aiVector3D nodePos( node.mNode->mTransformation.a4, node.mNode->mTransformation.b4, node.mNode->mTransformation.c4);
            nodeAnim->mNumPositionKeys = 1;
            nodeAnim->mPositionKeys = new aiVectorKey[1];
            nodeAnim->mPositionKeys[0].mTime = 0.0;
            nodeAnim->mPositionKeys[0].mValue = nodePos;
        }

        // rotation part. Always present. First find value offsets
        {
            unsigned int rotOffset  = 0;
            if( node.mChannels.size() == 6)
            {
                // Offset all further calculations
                rotOffset = 3;
            }

            // Then create the number of rotation keys
            nodeAnim->mNumRotationKeys = mAnimNumFrames;
            nodeAnim->mRotationKeys = new aiQuatKey[mAnimNumFrames];
            aiQuatKey* rotkey = nodeAnim->mRotationKeys;
            for( unsigned int fr = 0; fr < mAnimNumFrames; ++fr)
            {
                aiMatrix4x4 temp;
                aiMatrix3x3 rotMatrix;

                for( unsigned int channel = 0; channel < 3; ++channel)
                {
                    // translate ZXY euler angels into a quaternion
                    const float angle = node.mChannelValues[fr * node.mChannels.size() + rotOffset + channel] * float( AI_MATH_PI) / 180.0f;

                    // Compute rotation transformations in the right order
                    switch (node.mChannels[rotOffset+channel])
                    {
                    case Channel_RotationX: aiMatrix4x4::RotationX( angle, temp); rotMatrix *= aiMatrix3x3( temp); break;
                    case Channel_RotationY: aiMatrix4x4::RotationY( angle, temp); rotMatrix *= aiMatrix3x3( temp);  break;
                    case Channel_RotationZ: aiMatrix4x4::RotationZ( angle, temp); rotMatrix *= aiMatrix3x3( temp); break;
                    default: throw DeadlyImportError( "Unexpected animation channel setup at node " + nodeName );
                    }
                }

                rotkey->mTime = double( fr);
                rotkey->mValue = aiQuaternion( rotMatrix);
                ++rotkey;
            }
        }

        // scaling part. Always just a default track
        {
            nodeAnim->mNumScalingKeys = 1;
            nodeAnim->mScalingKeys = new aiVectorKey[1];
            nodeAnim->mScalingKeys[0].mTime = 0.0;
            nodeAnim->mScalingKeys[0].mValue.Set( 1.0f, 1.0f, 1.0f);
        }
    }
}

#endif // !! ASSIMP_BUILD_NO_BVH_IMPORTER
