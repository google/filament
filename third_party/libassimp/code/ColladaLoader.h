/** Defines the collada loader class */

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

#ifndef AI_COLLADALOADER_H_INC
#define AI_COLLADALOADER_H_INC

#include <assimp/BaseImporter.h>
#include "ColladaParser.h"

struct aiNode;
struct aiCamera;
struct aiLight;
struct aiTexture;
struct aiAnimation;

namespace Assimp
{

struct ColladaMeshIndex
{
    std::string mMeshID;
    size_t mSubMesh;
    std::string mMaterial;
    ColladaMeshIndex( const std::string& pMeshID, size_t pSubMesh, const std::string& pMaterial)
        : mMeshID( pMeshID), mSubMesh( pSubMesh), mMaterial( pMaterial)
    {   }

    bool operator < (const ColladaMeshIndex& p) const
    {
        if( mMeshID == p.mMeshID)
        {
            if( mSubMesh == p.mSubMesh)
                return mMaterial < p.mMaterial;
            else
                return mSubMesh < p.mSubMesh;
        } else
        {
            return mMeshID < p.mMeshID;
        }
    }
};

/** Loader class to read Collada scenes. Collada is over-engineered to death, with every new iteration bringing
 * more useless stuff, so I limited the data to what I think is useful for games.
*/
class ColladaLoader : public BaseImporter
{
public:
    ColladaLoader();
    ~ColladaLoader();


public:
    /** Returns whether the class can handle the format of the given file.
     * See BaseImporter::CanRead() for details. */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const;

protected:
    /** Return importer meta information.
     * See #BaseImporter::GetInfo for the details
     */
    const aiImporterDesc* GetInfo () const;

    void SetupProperties(const Importer* pImp);

    /** Imports the given file into the given scene structure.
     * See BaseImporter::InternReadFile() for details
     */
    void InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler);

    /** Recursively constructs a scene node for the given parser node and returns it. */
    aiNode* BuildHierarchy( const ColladaParser& pParser, const Collada::Node* pNode);

    /** Resolve node instances */
    void ResolveNodeInstances( const ColladaParser& pParser, const Collada::Node* pNode,
        std::vector<const Collada::Node*>& resolved);

    /** Builds meshes for the given node and references them */
    void BuildMeshesForNode( const ColladaParser& pParser, const Collada::Node* pNode,
        aiNode* pTarget);
		
    aiMesh *findMesh(std::string meshid);

    /** Creates a mesh for the given ColladaMesh face subset and returns the newly created mesh */
    aiMesh* CreateMesh( const ColladaParser& pParser, const Collada::Mesh* pSrcMesh, const Collada::SubMesh& pSubMesh,
        const Collada::Controller* pSrcController, size_t pStartVertex, size_t pStartFace);

    /** Builds cameras for the given node and references them */
    void BuildCamerasForNode( const ColladaParser& pParser, const Collada::Node* pNode,
        aiNode* pTarget);

    /** Builds lights for the given node and references them */
    void BuildLightsForNode( const ColladaParser& pParser, const Collada::Node* pNode,
        aiNode* pTarget);

    /** Stores all meshes in the given scene */
    void StoreSceneMeshes( aiScene* pScene);

    /** Stores all materials in the given scene */
    void StoreSceneMaterials( aiScene* pScene);

    /** Stores all lights in the given scene */
    void StoreSceneLights( aiScene* pScene);

    /** Stores all cameras in the given scene */
    void StoreSceneCameras( aiScene* pScene);

    /** Stores all textures in the given scene */
    void StoreSceneTextures( aiScene* pScene);

    /** Stores all animations
     * @param pScene target scene to store the anims
     */
    void StoreAnimations( aiScene* pScene, const ColladaParser& pParser);

    /** Stores all animations for the given source anim and its nested child animations
     * @param pScene target scene to store the anims
     * @param pSrcAnim the source animation to process
     * @param pPrefix Prefix to the name in case of nested animations
     */
    void StoreAnimations( aiScene* pScene, const ColladaParser& pParser, const Collada::Animation* pSrcAnim, const std::string& pPrefix);

    /** Constructs the animation for the given source anim */
    void CreateAnimation( aiScene* pScene, const ColladaParser& pParser, const Collada::Animation* pSrcAnim, const std::string& pName);

    /** Constructs materials from the collada material definitions */
    void BuildMaterials( ColladaParser& pParser, aiScene* pScene);

    /** Fill materials from the collada material definitions */
    void FillMaterials( const ColladaParser& pParser, aiScene* pScene);

    /** Resolve UV channel mappings*/
    void ApplyVertexToEffectSemanticMapping(Collada::Sampler& sampler,
        const Collada::SemanticMappingTable& table);

    /** Add a texture and all of its sampling properties to a material*/
    void AddTexture ( aiMaterial& mat, const ColladaParser& pParser,
        const Collada::Effect& effect,
        const Collada::Sampler& sampler,
        aiTextureType type, unsigned int idx = 0);

    /** Resolves the texture name for the given effect texture entry */
    aiString FindFilenameForEffectTexture( const ColladaParser& pParser,
        const Collada::Effect& pEffect, const std::string& pName);

    /** Converts a path read from a collada file to the usual representation */
    void ConvertPath( aiString& ss);

    /** Reads a float value from an accessor and its data array.
     * @param pAccessor The accessor to use for reading
     * @param pData The data array to read from
     * @param pIndex The index of the element to retrieve
     * @param pOffset Offset into the element, for multipart elements such as vectors or matrices
     * @return the specified value
     */
    ai_real ReadFloat( const Collada::Accessor& pAccessor, const Collada::Data& pData, size_t pIndex, size_t pOffset) const;

    /** Reads a string value from an accessor and its data array.
     * @param pAccessor The accessor to use for reading
     * @param pData The data array to read from
     * @param pIndex The index of the element to retrieve
     * @return the specified value
     */
    const std::string& ReadString( const Collada::Accessor& pAccessor, const Collada::Data& pData, size_t pIndex) const;

    /** Recursively collects all nodes into the given array */
    void CollectNodes( const aiNode* pNode, std::vector<const aiNode*>& poNodes) const;

    /** Finds a node in the collada scene by the given name */
    const Collada::Node* FindNode( const Collada::Node* pNode, const std::string& pName) const;
    /** Finds a node in the collada scene by the given SID */
    const Collada::Node* FindNodeBySID( const Collada::Node* pNode, const std::string& pSID) const;

    /** Finds a proper name for a node derived from the collada-node's properties */
    std::string FindNameForNode( const Collada::Node* pNode);

protected:
    /** Filename, for a verbose error message */
    std::string mFileName;

    /** Which mesh-material compound was stored under which mesh ID */
    std::map<ColladaMeshIndex, size_t> mMeshIndexByID;

    /** Which material was stored under which index in the scene */
    std::map<std::string, size_t> mMaterialIndexByName;

    /** Accumulated meshes for the target scene */
    std::vector<aiMesh*> mMeshes;
	
    /** Accumulated morph target meshes */
    std::vector<aiMesh*> mTargetMeshes;

    /** Temporary material list */
    std::vector<std::pair<Collada::Effect*, aiMaterial*> > newMats;

    /** Temporary camera list */
    std::vector<aiCamera*> mCameras;

    /** Temporary light list */
    std::vector<aiLight*> mLights;

    /** Temporary texture list */
    std::vector<aiTexture*> mTextures;

    /** Accumulated animations for the target scene */
    std::vector<aiAnimation*> mAnims;

    bool noSkeletonMesh;
    bool ignoreUpDirection;
    bool useColladaName;

    /** Used by FindNameForNode() to generate unique node names */
    unsigned int mNodeNameCounter;
};

} // end of namespace Assimp

#endif // AI_COLLADALOADER_H_INC
