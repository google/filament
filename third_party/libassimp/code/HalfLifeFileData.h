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


//
//! @file Definition of in-memory structures for the HL2 MDL file format
//  and for the HalfLife text format (SMD)
//
// The specification has been taken from various sources on the internet.


#ifndef AI_MDLFILEHELPER2_H_INC
#define AI_MDLFILEHELPER2_H_INC

#include <assimp/Compiler/pushpack1.h>

namespace Assimp    {
namespace MDL   {

// magic bytes used in Half Life 2 MDL models
#define AI_MDL_MAGIC_NUMBER_BE_HL2a AI_MAKE_MAGIC("IDST")
#define AI_MDL_MAGIC_NUMBER_LE_HL2a AI_MAKE_MAGIC("TSDI")
#define AI_MDL_MAGIC_NUMBER_BE_HL2b AI_MAKE_MAGIC("IDSQ")
#define AI_MDL_MAGIC_NUMBER_LE_HL2b AI_MAKE_MAGIC("QSDI")

// ---------------------------------------------------------------------------
/** \struct Header_HL2
 *  \brief Data structure for the HL2 main header
 */
// ---------------------------------------------------------------------------
struct Header_HL2 {
    //! magic number: "IDST"/"IDSQ"
    char    ident[4];

    //! Version number
    int32_t version;

    //! Original file name in pak ?
    char        name[64];

    //! Length of file name/length of file?
    int32_t     length;

    //! For viewer, ignored
    aiVector3D      eyeposition;
    aiVector3D      min;
    aiVector3D      max;

    //! AABB of the model
    aiVector3D      bbmin;
    aiVector3D      bbmax;

    // File flags
    int32_t         flags;

    //! NUmber of bones contained in the file
    int32_t         numbones;
    int32_t         boneindex;

    //! Number of bone controllers for bone animation
    int32_t         numbonecontrollers;
    int32_t         bonecontrollerindex;

    //! More bounding boxes ...
    int32_t         numhitboxes;
    int32_t         hitboxindex;

    //! Animation sequences in the file
    int32_t         numseq;
    int32_t         seqindex;

    //! Loaded sequences. Ignored
    int32_t         numseqgroups;
    int32_t         seqgroupindex;

    //! Raw texture data
    int32_t         numtextures;
    int32_t         textureindex;
    int32_t         texturedataindex;

    //! Number of skins (=textures?)
    int32_t         numskinref;
    int32_t         numskinfamilies;
    int32_t         skinindex;

    //! Number of parts
    int32_t         numbodyparts;
    int32_t         bodypartindex;

    //! attachable points for gameplay and physics
    int32_t         numattachments;
    int32_t         attachmentindex;

    //! Table of sound effects associated with the model
    int32_t         soundtable;
    int32_t         soundindex;
    int32_t         soundgroups;
    int32_t         soundgroupindex;

    //! Number of animation transitions
    int32_t         numtransitions;
    int32_t         transitionindex;
} /* PACK_STRUCT */;

#include <assimp/Compiler/poppack1.h>

}
} // end namespaces

#endif // ! AI_MDLFILEHELPER2_H_INC
