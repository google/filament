/*
---------------------------------------------------------------------------
Open Asset Import Library - Java Binding (jassimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2012, assimp team

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
package jassimp;


/**
 * Defines how the mapping coords for a texture are generated.<p>
 *
 * Real-time applications typically require full UV coordinates, so the use of
 * the {@link AiPostProcessSteps#GEN_UV_COORDS} step is highly recommended. 
 * It generates proper UV channels for non-UV mapped objects, as long as an 
 * accurate description how the mapping should look like (e.g spherical) is 
 * given.
 */
public enum AiTextureMapping {
       /** 
        * The mapping coordinates are taken from an UV channel.
        *
        *  The #AI_MATKEY_UVWSRC key specifies from which UV channel
        *  the texture coordinates are to be taken from (remember,
        *  meshes can have more than one UV channel). 
       */
//       aiTextureMapping_UV = 0x0,
//
//        /** Spherical mapping */
//       aiTextureMapping_SPHERE = 0x1,
//
//        /** Cylindrical mapping */
//       aiTextureMapping_CYLINDER = 0x2,
//
//        /** Cubic mapping */
//       aiTextureMapping_BOX = 0x3,
//
//        /** Planar mapping */
//       aiTextureMapping_PLANE = 0x4,
//
//        /** Undefined mapping. Have fun. */
//       aiTextureMapping_OTHER = 0x5,

}
