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

/** @file  BlenderModifier.h
 *  @brief Declare dedicated helper classes to simulate some blender modifiers (i.e. mirror)
 */
#ifndef INCLUDED_AI_BLEND_MODIFIER_H
#define INCLUDED_AI_BLEND_MODIFIER_H

#include "BlenderIntermediate.h"

namespace Assimp {
namespace Blender {

// -------------------------------------------------------------------------------------------
/** 
 *  Dummy base class for all blender modifiers. Modifiers are reused between imports, so
 *  they should be stateless and not try to cache model data. 
 */
// -------------------------------------------------------------------------------------------
class BlenderModifier {
public:
    /**
     *  The class destructor, virtual.
     */
    virtual ~BlenderModifier() {
        // empty
    }

    // --------------------
    /** 
     *  Check if *this* modifier is active, given a ModifierData& block.
     */
    virtual bool IsActive( const ModifierData& /*modin*/) {
        return false;
    }

    // --------------------
    /** 
     *  Apply the modifier to a given output node. The original data used
     *  to construct the node is given as well. Not called unless IsActive()
     *  was called and gave positive response. 
     */
    virtual void DoIt(aiNode& /*out*/,
        ConversionData& /*conv_data*/,
        const ElemBase& orig_modifier,
        const Scene& /*in*/,
        const Object& /*orig_object*/
    ) {
        ASSIMP_LOG_INFO_F("This modifier is not supported, skipping: ",orig_modifier.dna_type );
        return;
    }
};

// -------------------------------------------------------------------------------------------
/** 
 *  Manage all known modifiers and instance and apply them if necessary 
 */
// -------------------------------------------------------------------------------------------
class BlenderModifierShowcase {
public:
    // --------------------
    /** Apply all requested modifiers provided we support them. */
    void ApplyModifiers(aiNode& out,
        ConversionData& conv_data,
        const Scene& in,
        const Object& orig_object
    );

private:
    TempArray< std::vector,BlenderModifier > cached_modifiers;
};

// MODIFIERS /////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------
/** 
 *  Mirror modifier. Status: implemented. 
 */
// -------------------------------------------------------------------------------------------
class BlenderModifier_Mirror : public BlenderModifier {
public:
    // --------------------
    virtual bool IsActive( const ModifierData& modin);

    // --------------------
    virtual void DoIt(aiNode& out,
        ConversionData& conv_data,
        const ElemBase& orig_modifier,
        const Scene& in,
        const Object& orig_object
    ) ;
};

// -------------------------------------------------------------------------------------------
/** Subdivision modifier. Status: dummy. */
// -------------------------------------------------------------------------------------------
class BlenderModifier_Subdivision : public BlenderModifier {
public:

    // --------------------
    virtual bool IsActive( const ModifierData& modin);

    // --------------------
    virtual void DoIt(aiNode& out,
        ConversionData& conv_data,
        const ElemBase& orig_modifier,
        const Scene& in,
        const Object& orig_object
    ) ;
};

}
}

#endif // !INCLUDED_AI_BLEND_MODIFIER_H
