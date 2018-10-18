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

@author: Richard Steffen, 2014

----------------------------------------------------------------------
*/

/** @file XFileExporter.h
 * Declares the exporter class to write a scene to a Collada file
 */
#ifndef AI_XFILEEXPORTER_H_INC
#define AI_XFILEEXPORTER_H_INC

#include <assimp/ai_assert.h>
#include <assimp/matrix4x4.h>
#include <assimp/Exporter.hpp>
#include <sstream>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiString;

namespace Assimp {

class IOSystem;


/// Helper class to export a given scene to a X-file.
/// Note: an xFile uses a left hand system. Assimp used a right hand system (OpenGL), therefore we have to transform everything
class XFileExporter
{
public:
    /// Constructor for a specific scene to export
    XFileExporter(const aiScene* pScene, IOSystem* pIOSystem, const std::string& path, const std::string& file, const ExportProperties* pProperties);

    /// Destructor
    virtual ~XFileExporter();

protected:
    /// Starts writing the contents
    void WriteFile();

    /// Writes the asset header
    void WriteHeader();

    /// write a frame transform
    void WriteFrameTransform(aiMatrix4x4& m);

    /// Recursively writes the given node
    void WriteNode( aiNode* pNode );

    /// write a mesh entry of the scene
    void WriteMesh( aiMesh* mesh);

    /// Enters a new xml element, which increases the indentation
    void PushTag() { startstr.append( "  "); }

    /// Leaves an element, decreasing the indentation
    void PopTag() { 
        ai_assert( startstr.length() > 1); 
        startstr.erase( startstr.length() - 2); 
    }

public:
    /// Stringstream to write all output into
    std::stringstream mOutput;

protected:

    /// normalize the name to be accepted by xfile readers
    std::string toXFileString(aiString &name);

    /// hold the properties pointer
    const ExportProperties* mProperties;

    /// write a path
    void writePath(const aiString &path);

    /// The IOSystem for output
    IOSystem* mIOSystem;

    /// Path of the directory where the scene will be exported
    const std::string mPath;

    /// Name of the file (without extension) where the scene will be exported
    const std::string mFile;

    /// The scene to be written
    const aiScene* mScene;
    bool mSceneOwned;

    /// current line start string, contains the current indentation for simple stream insertion
    std::string startstr;

    /// current line end string for simple stream insertion
    std::string endstr;

};

}

#endif // !! AI_XFILEEXPORTER_H_INC
