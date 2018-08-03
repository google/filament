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

/** @file  Md3Loader.h
 *  @brief Declaration of the .MD3 importer class.
 */
#ifndef AI_MD3LOADER_H_INCLUDED
#define AI_MD3LOADER_H_INCLUDED

#include "BaseImporter.h"
#include "ByteSwapper.h"
#include "MD3FileData.h"
#include "StringComparison.h"
#include <assimp/types.h>

#include <list>

struct aiMaterial;

namespace Assimp    {


using namespace MD3;
namespace Q3Shader {

// ---------------------------------------------------------------------------
/** @brief Tiny utility data structure to hold the data of a .skin file
 */
struct SkinData
{
    //! A single entryin texture list
    struct TextureEntry : public std::pair<std::string,std::string>
    {
        // did we resolve this texture entry?
        bool resolved;

        // for std::find()
        bool operator == (const std::string& f) const {
            return f == first;
        }
    };

    //! List of textures
    std::list<TextureEntry> textures;

    // rest is ignored for the moment
};

// ---------------------------------------------------------------------------
/** @brief Specifies cull modi for Quake shader files.
 */
enum ShaderCullMode
{
    CULL_NONE,
    CULL_CW,
    CULL_CCW
};

// ---------------------------------------------------------------------------
/** @brief Specifies alpha blend modi (src + dest) for Quake shader files
 */
enum BlendFunc
{
    BLEND_NONE,
    BLEND_GL_ONE,
    BLEND_GL_ZERO,
    BLEND_GL_DST_COLOR,
    BLEND_GL_ONE_MINUS_DST_COLOR,
    BLEND_GL_SRC_ALPHA,
    BLEND_GL_ONE_MINUS_SRC_ALPHA
};

// ---------------------------------------------------------------------------
/** @brief Specifies alpha test modi for Quake texture maps
 */
enum AlphaTestFunc
{
    AT_NONE,
    AT_GT0,
    AT_LT128,
    AT_GE128
};

// ---------------------------------------------------------------------------
/** @brief Tiny utility data structure to hold a .shader map data block
 */
struct ShaderMapBlock
{
    ShaderMapBlock()
         :  blend_src   (BLEND_NONE)
         ,  blend_dest  (BLEND_NONE)
         ,  alpha_test  (AT_NONE)
    {}

    //! Name of referenced map
    std::string name;

    //! Blend and alpha test settings for texture
    BlendFunc blend_src,blend_dest;
    AlphaTestFunc alpha_test;


    //! For std::find()
    bool operator== (const std::string& o) const {
        return !ASSIMP_stricmp(o,name);
    }
};

// ---------------------------------------------------------------------------
/** @brief Tiny utility data structure to hold a .shader data block
 */
struct ShaderDataBlock
{
    ShaderDataBlock()
        :   cull    (CULL_CW)
    {}

    //! Name of referenced data element
    std::string name;

    //! Cull mode for the element
    ShaderCullMode cull;

    //! Maps defined in the shader
    std::list<ShaderMapBlock> maps;


    //! For std::find()
    bool operator== (const std::string& o) const {
        return !ASSIMP_stricmp(o,name);
    }
};

// ---------------------------------------------------------------------------
/** @brief Tiny utility data structure to hold the data of a .shader file
 */
struct ShaderData
{
    //! Shader data blocks
    std::list<ShaderDataBlock> blocks;
};

// ---------------------------------------------------------------------------
/** @brief Load a shader file
 *
 *  Generally, parsing is error tolerant. There's no failure.
 *  @param fill Receives output data
 *  @param file File to be read.
 *  @param io IOSystem to be used for reading
 *  @return false if file is not accessible
 */
bool LoadShader(ShaderData& fill, const std::string& file,IOSystem* io);


// ---------------------------------------------------------------------------
/** @brief Convert a Q3Shader to an aiMaterial
 *
 *  @param[out] out Material structure to be filled.
 *  @param[in] shader Input shader
 */
void ConvertShaderToMaterial(aiMaterial* out, const ShaderDataBlock& shader);

// ---------------------------------------------------------------------------
/** @brief Load a skin file
 *
 *  Generally, parsing is error tolerant. There's no failure.
 *  @param fill Receives output data
 *  @param file File to be read.
 *  @param io IOSystem to be used for reading
 *  @return false if file is not accessible
 */
bool LoadSkin(SkinData& fill, const std::string& file,IOSystem* io);

} // ! namespace Q3SHader

// ---------------------------------------------------------------------------
/** @brief Importer class to load MD3 files
*/
class MD3Importer : public BaseImporter
{
public:
    MD3Importer();
    ~MD3Importer();


public:

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
    /** Validate offsets in the header
     */
    void ValidateHeaderOffsets();
    void ValidateSurfaceHeaderOffsets(const MD3::Surface* pcSurfHeader);

    // -------------------------------------------------------------------
    /** Read a Q3 multipart file
     *  @return true if multi part has been processed
     */
    bool ReadMultipartFile();

    // -------------------------------------------------------------------
    /** Try to read the skin for a MD3 file
     *  @param fill Receives output information
     */
    void ReadSkin(Q3Shader::SkinData& fill) const;

    // -------------------------------------------------------------------
    /** Try to read the shader for a MD3 file
     *  @param fill Receives output information
     */
    void ReadShader(Q3Shader::ShaderData& fill) const;

    // -------------------------------------------------------------------
    /** Convert a texture path in a MD3 file to a proper value
     *  @param[in] texture_name Path to be converted
     *  @param[in] header_path Base path specified in MD3 header
     *  @param[out] out Receives the converted output string
     */
    void ConvertPath(const char* texture_name, const char* header_path,
        std::string& out) const;

protected:

    /** Configuration option: frame to be loaded */
    unsigned int configFrameID;

    /** Configuration option: process multi-part files */
    bool configHandleMP;

    /** Configuration option: name of skin file to be read */
    std::string configSkinFile;

    /** Configuration option: name or path of shader */
    std::string configShaderFile;

    /** Configuration option: speed flag was set? */
    bool configSpeedFlag;

    /** Header of the MD3 file */
    BE_NCONST MD3::Header* pcHeader;

    /** File buffer  */
    BE_NCONST unsigned char* mBuffer;

    /** Size of the file, in bytes */
    unsigned int fileSize;

    /** Current file name */
    std::string mFile;

    /** Current base directory  */
    std::string path;

    /** Pure file we're currently reading */
    std::string filename;

    /** Output scene to be filled */
    aiScene* mScene;

    /** IO system to be used to access the data*/
    IOSystem* mIOHandler;
    };

} // end of namespace Assimp

#endif // AI_3DSIMPORTER_H_INC
