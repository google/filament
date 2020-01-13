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

/** @file  BlenderIntermediate.h
 *  @brief Internal utility structures for the BlenderLoader. It also serves
 *    as master include file for the whole (internal) Blender subsystem.
 */
#ifndef INCLUDED_AI_BLEND_INTERMEDIATE_H
#define INCLUDED_AI_BLEND_INTERMEDIATE_H

#include "BlenderLoader.h"
#include "BlenderDNA.h"
#include "BlenderScene.h"
#include <deque>
#include <assimp/material.h>

struct aiTexture;

namespace Assimp {
namespace Blender {

    // --------------------------------------------------------------------
    /** Mini smart-array to avoid pulling in even more boost stuff. usable with vector and deque */
    // --------------------------------------------------------------------
    template <template <typename,typename> class TCLASS, typename T>
    struct TempArray    {
        typedef TCLASS< T*,std::allocator<T*> > mywrap;

        TempArray() {
        }

        ~TempArray () {
            for(T* elem : arr) {
                delete elem;
            }
        }

        void dismiss() {
            arr.clear();
        }

        mywrap* operator -> () {
            return &arr;
        }

        operator mywrap& () {
            return arr;
        }

        operator const mywrap& () const {
            return arr;
        }

        mywrap& get () {
            return arr;
        }

        const mywrap& get () const {
            return arr;
        }

        T* operator[] (size_t idx) const {
            return arr[idx];
        }

        T*& operator[] (size_t idx) {
            return arr[idx];
        }

    private:
        // no copy semantics
        void operator= (const TempArray&)  {
        }

        TempArray(const TempArray& /*arr*/) {
        }

    private:
        mywrap arr;
    };

#ifdef _MSC_VER
#   pragma warning(disable:4351)
#endif

    // As counter-intuitive as it may seem, a comparator must return false for equal values.
    // The C++ standard defines and expects this behavior: true if lhs < rhs, false otherwise.
    struct ObjectCompare {
        bool operator() (const Object* left, const Object* right) const {
            return ::strncmp(left->id.name, right->id.name, strlen( left->id.name ) ) < 0;
        }
    };

    // When keeping objects in sets, sort them by their name.
    typedef std::set<const Object*, ObjectCompare> ObjectSet;

    // --------------------------------------------------------------------
    /** ConversionData acts as intermediate storage location for
     *  the various ConvertXXX routines in BlenderImporter.*/
    // --------------------------------------------------------------------
    struct ConversionData
    {
        ConversionData(const FileDatabase& db)
            : sentinel_cnt()
            , next_texture()
            , db(db)
        {}

        // As counter-intuitive as it may seem, a comparator must return false for equal values.
        // The C++ standard defines and expects this behavior: true if lhs < rhs, false otherwise.
        struct ObjectCompare {
            bool operator() (const Object* left, const Object* right) const {
                return ::strncmp( left->id.name, right->id.name, strlen( left->id.name ) ) < 0;
            }
        };

        ObjectSet objects;

        TempArray <std::vector, aiMesh> meshes;
        TempArray <std::vector, aiCamera> cameras;
        TempArray <std::vector, aiLight> lights;
        TempArray <std::vector, aiMaterial> materials;
        TempArray <std::vector, aiTexture> textures;

        // set of all materials referenced by at least one mesh in the scene
        std::deque< std::shared_ptr< Material > > materials_raw;

        // counter to name sentinel textures inserted as substitutes for procedural textures.
        unsigned int sentinel_cnt;

        // next texture ID for each texture type, respectively
        unsigned int next_texture[aiTextureType_UNKNOWN+1];

        // original file data
        const FileDatabase& db;
    };
#ifdef _MSC_VER
#   pragma warning(default:4351)
#endif

// ------------------------------------------------------------------------------------------------
inline const char* GetTextureTypeDisplayString(Tex::Type t)
{
    switch (t)  {
    case Tex::Type_CLOUDS       :  return  "Clouds";
    case Tex::Type_WOOD         :  return  "Wood";
    case Tex::Type_MARBLE       :  return  "Marble";
    case Tex::Type_MAGIC        :  return  "Magic";
    case Tex::Type_BLEND        :  return  "Blend";
    case Tex::Type_STUCCI       :  return  "Stucci";
    case Tex::Type_NOISE        :  return  "Noise";
    case Tex::Type_PLUGIN       :  return  "Plugin";
    case Tex::Type_MUSGRAVE     :  return  "Musgrave";
    case Tex::Type_VORONOI      :  return  "Voronoi";
    case Tex::Type_DISTNOISE    :  return  "DistortedNoise";
    case Tex::Type_ENVMAP       :  return  "EnvMap";
    case Tex::Type_IMAGE        :  return  "Image";
    default:
        break;
    }
    return "<Unknown>";
}

} // ! Blender
} // ! Assimp

#endif // ! INCLUDED_AI_BLEND_INTERMEDIATE_H
