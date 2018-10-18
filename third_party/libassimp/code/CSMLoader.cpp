/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team



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

/** @file  CSMLoader.cpp
 *  Implementation of the CSM importer class.
 */



#ifndef ASSIMP_BUILD_NO_CSM_IMPORTER

#include "CSMLoader.h"
#include <assimp/SkeletonMeshBuilder.h>
#include <assimp/ParsingUtils.h>
#include <assimp/fast_atof.h>
#include <assimp/Importer.hpp>
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/anim.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/importerdesc.h>

using namespace Assimp;

static const aiImporterDesc desc = {
    "CharacterStudio Motion Importer (MoCap)",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour,
    0,
    0,
    0,
    0,
    "csm"
};


// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
CSMImporter::CSMImporter()
: noSkeletonMesh()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
CSMImporter::~CSMImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool CSMImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    // check file extension
    const std::string extension = GetExtension(pFile);

    if( extension == "csm")
        return true;

    if ((checkSig || !extension.length()) && pIOHandler) {
        const char* tokens[] = {"$Filename"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Build a string of all file extensions supported
const aiImporterDesc* CSMImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup configuration properties for the loader
void CSMImporter::SetupProperties(const Importer* pImp)
{
    noSkeletonMesh = pImp->GetPropertyInteger(AI_CONFIG_IMPORT_NO_SKELETON_MESHES,0) != 0;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void CSMImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL) {
        throw DeadlyImportError( "Failed to open CSM file " + pFile + ".");
    }

    // allocate storage and copy the contents of the file to a memory buffer
    std::vector<char> mBuffer2;
    TextFileToBuffer(file.get(),mBuffer2);
    const char* buffer = &mBuffer2[0];

    std::unique_ptr<aiAnimation> anim(new aiAnimation());
    int first = 0, last = 0x00ffffff;

    // now process the file and look out for '$' sections
    while (1)   {
        SkipSpaces(&buffer);
        if ('\0' == *buffer)
            break;

        if ('$'  == *buffer)    {
            ++buffer;
            if (TokenMatchI(buffer,"firstframe",10))    {
                SkipSpaces(&buffer);
                first = strtol10(buffer,&buffer);
            }
            else if (TokenMatchI(buffer,"lastframe",9))     {
                SkipSpaces(&buffer);
                last = strtol10(buffer,&buffer);
            }
            else if (TokenMatchI(buffer,"rate",4))  {
                SkipSpaces(&buffer);
                float d;
                buffer = fast_atoreal_move<float>(buffer,d);
                anim->mTicksPerSecond = d;
            }
            else if (TokenMatchI(buffer,"order",5)) {
                std::vector< aiNodeAnim* > anims_temp;
                anims_temp.reserve(30);
                while (1)   {
                    SkipSpaces(&buffer);
                    if (IsLineEnd(*buffer) && SkipSpacesAndLineEnd(&buffer) && *buffer == '$')
                        break; // next section

                    // Construct a new node animation channel and setup its name
                    anims_temp.push_back(new aiNodeAnim());
                    aiNodeAnim* nda = anims_temp.back();

                    char* ot = nda->mNodeName.data;
                    while (!IsSpaceOrNewLine(*buffer))
                        *ot++ = *buffer++;

                    *ot = '\0';
                    nda->mNodeName.length = (size_t)(ot-nda->mNodeName.data);
                }

                anim->mNumChannels = static_cast<unsigned int>(anims_temp.size());
                if (!anim->mNumChannels)
                    throw DeadlyImportError("CSM: Empty $order section");

                // copy over to the output animation
                anim->mChannels = new aiNodeAnim*[anim->mNumChannels];
                ::memcpy(anim->mChannels,&anims_temp[0],sizeof(aiNodeAnim*)*anim->mNumChannels);
            }
            else if (TokenMatchI(buffer,"points",6))    {
                if (!anim->mNumChannels)
                    throw DeadlyImportError("CSM: \'$order\' section is required to appear prior to \'$points\'");

                // If we know how many frames we'll read, we can preallocate some storage
                unsigned int alloc = 100;
                if (last != 0x00ffffff)
                {
                    alloc = last-first;
                    alloc += alloc>>2u; // + 25%
                    for (unsigned int i = 0; i < anim->mNumChannels;++i)
                        anim->mChannels[i]->mPositionKeys = new aiVectorKey[alloc];
                }

                unsigned int filled = 0;

                // Now read all point data.
                while (1)   {
                    SkipSpaces(&buffer);
                    if (IsLineEnd(*buffer) && (!SkipSpacesAndLineEnd(&buffer) || *buffer == '$'))   {
                        break; // next section
                    }

                    // read frame
                    const int frame = ::strtoul10(buffer,&buffer);
                    last  = std::max(frame,last);
                    first = std::min(frame,last);
                    for (unsigned int i = 0; i < anim->mNumChannels;++i)    {

                        aiNodeAnim* s = anim->mChannels[i];
                        if (s->mNumPositionKeys == alloc)   { /* need to reallocate? */

                            aiVectorKey* old = s->mPositionKeys;
                            s->mPositionKeys = new aiVectorKey[s->mNumPositionKeys = alloc*2];
                            ::memcpy(s->mPositionKeys,old,sizeof(aiVectorKey)*alloc);
                            delete[] old;
                        }

                        // read x,y,z
                        if(!SkipSpacesAndLineEnd(&buffer))
                            throw DeadlyImportError("CSM: Unexpected EOF occurred reading sample x coord");

                        if (TokenMatchI(buffer, "DROPOUT", 7))  {
                            // seems this is invalid marker data; at least the doc says it's possible
                            ASSIMP_LOG_WARN("CSM: Encountered invalid marker data (DROPOUT)");
                        }
                        else    {
                            aiVectorKey* sub = s->mPositionKeys + s->mNumPositionKeys;
                            sub->mTime = (double)frame;
                            buffer = fast_atoreal_move<float>(buffer, (float&)sub->mValue.x);

                            if(!SkipSpacesAndLineEnd(&buffer))
                                throw DeadlyImportError("CSM: Unexpected EOF occurred reading sample y coord");
                            buffer = fast_atoreal_move<float>(buffer, (float&)sub->mValue.y);

                            if(!SkipSpacesAndLineEnd(&buffer))
                                throw DeadlyImportError("CSM: Unexpected EOF occurred reading sample z coord");
                            buffer = fast_atoreal_move<float>(buffer, (float&)sub->mValue.z);

                            ++s->mNumPositionKeys;
                        }
                    }

                    // update allocation granularity
                    if (filled == alloc)
                        alloc *= 2;

                    ++filled;
                }
                // all channels must be complete in order to continue safely.
                for (unsigned int i = 0; i < anim->mNumChannels;++i)    {

                    if (!anim->mChannels[i]->mNumPositionKeys)
                        throw DeadlyImportError("CSM: Invalid marker track");
                }
            }
        }
        else    {
            // advance to the next line
            SkipLine(&buffer);
        }
    }

    // Setup a proper animation duration
    anim->mDuration = last - std::min( first, 0 );

    // build a dummy root node with the tiny markers as children
    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mName.Set("$CSM_DummyRoot");

    pScene->mRootNode->mNumChildren = anim->mNumChannels;
    pScene->mRootNode->mChildren = new aiNode* [anim->mNumChannels];

    for (unsigned int i = 0; i < anim->mNumChannels;++i)    {
        aiNodeAnim* na = anim->mChannels[i];

        aiNode* nd  = pScene->mRootNode->mChildren[i] = new aiNode();
        nd->mName   = anim->mChannels[i]->mNodeName;
        nd->mParent = pScene->mRootNode;

        aiMatrix4x4::Translation(na->mPositionKeys[0].mValue, nd->mTransformation);
    }

    // Store the one and only animation in the scene
    pScene->mAnimations    = new aiAnimation*[pScene->mNumAnimations=1];
    anim->mName.Set("$CSM_MasterAnim");
    pScene->mAnimations[0] = anim.release();

    // mark the scene as incomplete and run SkeletonMeshBuilder on it
    pScene->mFlags |= AI_SCENE_FLAGS_INCOMPLETE;

    if (!noSkeletonMesh) {
        SkeletonMeshBuilder maker(pScene,pScene->mRootNode,true);
    }
}

#endif // !! ASSIMP_BUILD_NO_CSM_IMPORTER
