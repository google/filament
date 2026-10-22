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


/**  @file MDLLoader.h
 *   @brief Declaration of the loader for MDL files
 */

#ifndef AI_MDLLOADER_H_INCLUDED
#define AI_MDLLOADER_H_INCLUDED

#include <assimp/BaseImporter.h>
#include "MDLFileData.h"
#include "HMP/HalfLifeFileData.h"

struct aiNode;
struct aiTexture;

namespace Assimp    {


using namespace MDL;

// --------------------------------------------------------------------------------------
// Include file/line information in debug builds
#ifdef ASSIMP_BUILD_DEBUG
#   define VALIDATE_FILE_SIZE(msg) SizeCheck(msg,__FILE__,__LINE__)
#else
#   define VALIDATE_FILE_SIZE(msg) SizeCheck(msg)
#endif

// --------------------------------------------------------------------------------------
/** @brief Class to load MDL files.
 *
 *  Several subformats exist:
 *   <ul>
 *      <li>Quake I</li>
 *      <li>3D Game Studio MDL3, MDL4</li>
 *      <li>3D Game Studio MDL5</li>
 *      <li>3D Game Studio MDL7</li>
 *      <li>Halflife 2</li>
 *   </ul>
 *  These formats are partially identical and it would be possible to load
 *  them all with a single 1000-line function-beast. However, it has been
 *  split into several code paths to make the code easier to read and maintain.
*/
class MDLImporter : public BaseImporter
{
public:
    MDLImporter();
    ~MDLImporter();

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
    * See BaseImporter::CanRead() for details.  */
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

    // -------------------------------------------------------------------
    /** Import a quake 1 MDL file (IDPO)
    */
    void InternReadFile_Quake1( );

    // -------------------------------------------------------------------
    /** Import a GameStudio A4/A5 file (MDL 3,4,5)
    */
    void InternReadFile_3DGS_MDL345( );

    // -------------------------------------------------------------------
    /** Import a GameStudio A7 file (MDL 7)
    */
    void InternReadFile_3DGS_MDL7( );

    // -------------------------------------------------------------------
    /** Import a CS:S/HL2 MDL file (not fully implemented)
    */
    void InternReadFile_HL2( );

    // -------------------------------------------------------------------
    /** Check whether a given position is inside the valid range
     *  Throw a DeadlyImportError if it is not
     * \param szPos Cursor position
     * \param szFile Name of the source file from which the function was called
     * \param iLine Source code line from which the function was called
    */
    void SizeCheck(const void* szPos);
    void SizeCheck(const void* szPos, const char* szFile, unsigned int iLine);

    // -------------------------------------------------------------------
    /** Validate the header data structure of a game studio MDL7 file
     * \param pcHeader Input header to be validated
     */
    void ValidateHeader_3DGS_MDL7(const MDL::Header_MDL7* pcHeader);

    // -------------------------------------------------------------------
    /** Validate the header data structure of a Quake 1 model
     * \param pcHeader Input header to be validated
     */
    void ValidateHeader_Quake1(const MDL::Header* pcHeader);

    // -------------------------------------------------------------------
    /** Try to load a  palette from the current directory (colormap.lmp)
     *  If it is not found the default palette of Quake1 is returned
     */
    void SearchPalette(const unsigned char** pszColorMap);

    // -------------------------------------------------------------------
    /** Free a palette created with a previous call to SearchPalette()
     */
    void FreePalette(const unsigned char* pszColorMap);

    // -------------------------------------------------------------------
    /** Load a palletized texture from the file and convert it to 32bpp
    */
    void CreateTextureARGB8_3DGS_MDL3(const unsigned char* szData);

    // -------------------------------------------------------------------
    /** Used to load textures from MDL3/4
     * \param szData Input data
     * \param iType Color data type
     * \param piSkip Receive: Size to skip, in bytes
    */
    void CreateTexture_3DGS_MDL4(const unsigned char* szData,
        unsigned int iType,
        unsigned int* piSkip);

    // -------------------------------------------------------------------
    /** Used to load textures from MDL5
     * \param szData Input data
     * \param iType Color data type
     * \param piSkip Receive: Size to skip, in bytes
    */
    void CreateTexture_3DGS_MDL5(const unsigned char* szData,
        unsigned int iType,
        unsigned int* piSkip);

    // -------------------------------------------------------------------
    /** Checks whether a texture can be replaced with a single color
     * This is useful for all file formats before MDL7 (all those
     * that are not containing material colors separate from textures).
     * MED seems to write dummy 8x8 monochrome images instead.
     * \param pcTexture Input texture
     * \return aiColor.r is set to qnan if the function fails and no
     *   color can be found.
    */
    aiColor4D ReplaceTextureWithColor(const aiTexture* pcTexture);

    // -------------------------------------------------------------------
    /** Converts the absolute texture coordinates in MDL5 files to
     *  relative in a range between 0 and 1
    */
    void CalculateUVCoordinates_MDL5();

    // -------------------------------------------------------------------
    /** Read an UV coordinate from the file. If the file format is not
     * MDL5, the function calculates relative texture coordinates
     * \param vOut Receives the output UV coord
     * \param pcSrc UV coordinate buffer
     * \param UV coordinate index
    */
    void ImportUVCoordinate_3DGS_MDL345( aiVector3D& vOut,
        const MDL::TexCoord_MDL3* pcSrc,
        unsigned int iIndex);

    // -------------------------------------------------------------------
    /** Setup the material properties for Quake and MDL<7 models.
     * These formats don't support more than one material per mesh,
     * therefore the method processes only ONE skin and removes
     * all others.
     */
    void SetupMaterialProperties_3DGS_MDL5_Quake1( );

    // -------------------------------------------------------------------
    /** Parse a skin lump in a MDL7/HMP7 file with all of its features
     *  variant 1: Current cursor position is the beginning of the skin header
     * \param szCurrent Current data pointer
     * \param szCurrentOut Output data pointer
     * \param pcMats Material list for this group. To be filled ...
     */
    void ParseSkinLump_3DGS_MDL7(
        const unsigned char* szCurrent,
        const unsigned char** szCurrentOut,
        std::vector<aiMaterial*>& pcMats);

    // -------------------------------------------------------------------
    /** Parse a skin lump in a MDL7/HMP7 file with all of its features
     *  variant 2: Current cursor position is the beginning of the skin data
     * \param szCurrent Current data pointer
     * \param szCurrentOut Output data pointer
     * \param pcMatOut Output material
     * \param iType header.typ
     * \param iWidth header.width
     * \param iHeight header.height
     */
    void ParseSkinLump_3DGS_MDL7(
        const unsigned char* szCurrent,
        const unsigned char** szCurrentOut,
        aiMaterial* pcMatOut,
        unsigned int iType,
        unsigned int iWidth,
        unsigned int iHeight);

    // -------------------------------------------------------------------
    /** Skip a skin lump in a MDL7/HMP7 file
     * \param szCurrent Current data pointer
     * \param szCurrentOut Output data pointer. Points to the byte just
     * behind the last byte of the skin.
     * \param iType header.typ
     * \param iWidth header.width
     * \param iHeight header.height
     */
    void SkipSkinLump_3DGS_MDL7(const unsigned char* szCurrent,
        const unsigned char** szCurrentOut,
        unsigned int iType,
        unsigned int iWidth,
        unsigned int iHeight);

    // -------------------------------------------------------------------
    /** Parse texture color data for MDL5, MDL6 and MDL7 formats
     * \param szData Current data pointer
     * \param iType type of the texture data. No DDS or external
     * \param piSkip Receive the number of bytes to skip
     * \param pcNew Must point to fully initialized data. Width and
     *        height must be set. If pcNew->pcData is set to UINT_MAX,
     *        piSkip will receive the size of the texture, in bytes, but no
     *        color data will be read.
     */
    void ParseTextureColorData(const unsigned char* szData,
        unsigned int iType,
        unsigned int* piSkip,
        aiTexture* pcNew);

    // -------------------------------------------------------------------
    /** Join two materials / skins. Setup UV source ... etc
     * \param pcMat1 First input material
     * \param pcMat2 Second input material
     * \param pcMatOut Output material instance to be filled. Must be empty
     */
    void JoinSkins_3DGS_MDL7(aiMaterial* pcMat1,
        aiMaterial* pcMat2,
        aiMaterial* pcMatOut);

    // -------------------------------------------------------------------
    /** Add a bone transformation key to an animation
     * \param iTrafo Index of the transformation (always==frame index?)
     * No need to validate this index, it is always valid.
     * \param pcBoneTransforms Bone transformation for this index
     * \param apcOutBones Output bones array
     */
    void AddAnimationBoneTrafoKey_3DGS_MDL7(unsigned int iTrafo,
        const MDL::BoneTransform_MDL7* pcBoneTransforms,
        MDL::IntBone_MDL7** apcBonesOut);

    // -------------------------------------------------------------------
    /** Load the bone list of a MDL7 file
     * \return If the bones could be loaded successfully, a valid
     *   array containing pointers to a temporary bone
     *   representation. NULL if the bones could not be loaded.
     */
    MDL::IntBone_MDL7** LoadBones_3DGS_MDL7();

    // -------------------------------------------------------------------
    /** Load bone transformation keyframes from a file chunk
     * \param groupInfo -> doc of data structure
     * \param frame -> doc of data structure
     * \param shared -> doc of data structure
     */
    void ParseBoneTrafoKeys_3DGS_MDL7(
        const MDL::IntGroupInfo_MDL7& groupInfo,
        IntFrameInfo_MDL7& frame,
        MDL::IntSharedData_MDL7& shared);

    // -------------------------------------------------------------------
    /** Calculate absolute bone animation matrices for each bone
     * \param apcOutBones Output bones array
     */
    void CalcAbsBoneMatrices_3DGS_MDL7(MDL::IntBone_MDL7** apcOutBones);

    // -------------------------------------------------------------------
    /** Add all bones to the nodegraph (as children of the root node)
     * \param apcBonesOut List of bones
     * \param pcParent Parent node. New nodes will be added to this node
     * \param iParentIndex Index of the parent bone
     */
    void AddBonesToNodeGraph_3DGS_MDL7(const MDL::IntBone_MDL7** apcBonesOut,
        aiNode* pcParent,uint16_t iParentIndex);

    // -------------------------------------------------------------------
    /** Build output animations
     * \param apcBonesOut List of bones
     */
    void BuildOutputAnims_3DGS_MDL7(const MDL::IntBone_MDL7** apcBonesOut);

    // -------------------------------------------------------------------
    /** Handles materials that are just referencing another material
     * There is no test file for this feature, but Conitec's doc
     * say it is used.
     */
    void HandleMaterialReferences_3DGS_MDL7();

    // -------------------------------------------------------------------
    /** Copies only the material that are referenced by at least one
     * mesh to the final output material list. All other materials
     * will be discarded.
     * \param shared -> doc of data structure
     */
    void CopyMaterials_3DGS_MDL7(MDL::IntSharedData_MDL7 &shared);

    // -------------------------------------------------------------------
    /** Process the frame section at the end of a group
     * \param groupInfo -> doc of data structure
     * \param shared -> doc of data structure
     * \param szCurrent Pointer to the start of the frame section
     * \param szCurrentOut Receives a pointer to the first byte of the
     *   next data section.
     * \return false to read no further groups (a small workaround for
     *   some tiny and unsolved problems ... )
     */
    bool ProcessFrames_3DGS_MDL7(const MDL::IntGroupInfo_MDL7& groupInfo,
        MDL::IntGroupData_MDL7& groupData,
        MDL::IntSharedData_MDL7& shared,
        const unsigned char* szCurrent,
        const unsigned char** szCurrentOut);

    // -------------------------------------------------------------------
    /** Sort all faces by their materials. If the mesh is using
     * multiple materials per face (that are blended together) the function
     * might create new materials.
     * \param groupInfo -> doc of data structure
     * \param groupData -> doc of data structure
     * \param splitGroupData -> doc of data structure
     */
    void SortByMaterials_3DGS_MDL7(
        const MDL::IntGroupInfo_MDL7& groupInfo,
        MDL::IntGroupData_MDL7& groupData,
        MDL::IntSplitGroupData_MDL7& splitGroupData);

    // -------------------------------------------------------------------
    /** Read all faces and vertices from a MDL7 group. The function fills
     *  preallocated memory buffers.
     * \param groupInfo -> doc of data structure
     * \param groupData -> doc of data structure
     */
    void ReadFaces_3DGS_MDL7(const MDL::IntGroupInfo_MDL7& groupInfo,
        MDL::IntGroupData_MDL7& groupData);

    // -------------------------------------------------------------------
    /** Generate the final output meshes for a7 models
     * \param groupData -> doc of data structure
     * \param splitGroupData -> doc of data structure
     */
    void GenerateOutputMeshes_3DGS_MDL7(
        MDL::IntGroupData_MDL7& groupData,
        MDL::IntSplitGroupData_MDL7& splitGroupData);

protected:

    /** Configuration option: frame to be loaded */
    unsigned int configFrameID;

    /** Configuration option: palette to be used to decode palletized images*/
    std::string configPalette;

    /** Buffer to hold the loaded file */
    unsigned char* mBuffer;

    /** For GameStudio MDL files: The number in the magic word, either 3,4 or 5
     * (MDL7 doesn't need this, the format has a separate loader) */
    unsigned int iGSFileVersion;

    /** Output I/O handler. used to load external lmp files */
    IOSystem* pIOHandler;

    /** Output scene to be filled */
    aiScene* pScene;

    /** Size of the input file in bytes */
    unsigned int iFileSize;
};

} // end of namespace Assimp

#endif // AI_3DSIMPORTER_H_INC
