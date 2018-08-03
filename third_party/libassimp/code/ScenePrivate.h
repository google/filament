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

/** @file Stuff to deal with aiScene::mPrivate
 */
#ifndef AI_SCENEPRIVATE_H_INCLUDED
#define AI_SCENEPRIVATE_H_INCLUDED

#include <assimp/scene.h>

namespace Assimp    {

class Importer;

struct ScenePrivateData {

    ScenePrivateData()
        : mOrigImporter()
        , mPPStepsApplied()
        , mIsCopy()
    {}

    // Importer that originally loaded the scene though the C-API
    // If set, this object is owned by this private data instance.
    Assimp::Importer* mOrigImporter;

    // List of post-processing steps already applied to the scene.
    unsigned int mPPStepsApplied;

    // true if the scene is a copy made with aiCopyScene()
    // or the corresponding C++ API. This means that user code
    // may have made modifications to it, so mPPStepsApplied
    // and mOrigImporter are no longer safe to rely on and only
    // serve informative purposes.
    bool mIsCopy;
};

// Access private data stored in the scene
inline ScenePrivateData* ScenePriv(aiScene* in) {
    return static_cast<ScenePrivateData*>(in->mPrivate);
}

inline const ScenePrivateData* ScenePriv(const aiScene* in) {
    return static_cast<const ScenePrivateData*>(in->mPrivate);
}

}

#endif
