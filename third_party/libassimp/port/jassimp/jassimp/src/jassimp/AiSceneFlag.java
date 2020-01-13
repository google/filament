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

import java.util.Set;


/**
 * Status flags for {@link AiScene}s.
 */
public enum AiSceneFlag {
    /**
     * Specifies that the scene data structure that was imported is not 
     * complete.<p>
     * 
     * This flag bypasses some internal validations and allows the import 
     * of animation skeletons, material libraries or camera animation paths 
     * using Assimp. Most applications won't support such data. 
     */
    INCOMPLETE(0x1),

    
    /**
     * This flag is set by the validation
     * ({@link AiPostProcessSteps#VALIDATE_DATA_STRUCTURE 
     *      VALIDATE_DATA_STRUCTURE})
     * postprocess-step if the validation is successful.<p> 
     * 
     * In a validated scene you can be sure that any cross references in the 
     * data structure (e.g. vertex indices) are valid.
     */
    VALIDATED(0x2),

    
    /**
     * * This flag is set by the validation
     * ({@link AiPostProcessSteps#VALIDATE_DATA_STRUCTURE 
     *      VALIDATE_DATA_STRUCTURE})
     * postprocess-step if the validation is successful but some issues have 
     * been found.<p>
     * 
     * This can for example mean that a texture that does not exist is 
     * referenced by a material or that the bone weights for a vertex don't sum
     * to 1.0 ... . In most cases you should still be able to use the import. 
     * This flag could be useful for applications which don't capture Assimp's 
     * log output.
     */
    VALIDATION_WARNING(0x4),

    
    /**
     * This flag is currently only set by the 
     * {@link jassimp.AiPostProcessSteps#JOIN_IDENTICAL_VERTICES 
     * JOIN_IDENTICAL_VERTICES}.<p>
     * 
     * It indicates that the vertices of the output meshes aren't in the 
     * internal verbose format anymore. In the verbose format all vertices are
     * unique, no vertex is ever referenced by more than one face.
     */
    NON_VERBOSE_FORMAT(0x8),
    

     /**
     * Denotes pure height-map terrain data.<p>
     * 
     * Pure terrains usually consist of quads, sometimes triangles, in a 
     * regular grid. The x,y coordinates of all vertex positions refer to the 
     * x,y coordinates on the terrain height map, the z-axis stores the 
     * elevation at a specific point.<p>
     *
     * TER (Terragen) and HMP (3D Game Studio) are height map formats.
     * <p>
     * Assimp is probably not the best choice for loading *huge* terrains -
     * fully triangulated data takes extremely much free store and should be 
     * avoided as long as possible (typically you'll do the triangulation when 
     * you actually need to render it).
     */
    TERRAIN(0x10);

    /**
     * The mapped c/c++ integer enum value.
     */
    private final int m_rawValue;

    /**
     * Utility method for converting from c/c++ based integer enums to java 
     * enums.<p>
     * 
     * This method is intended to be used from JNI and my change based on
     * implementation needs.
     * 
     * @param set the target set to fill
     * @param rawValue an integer based enum value (as defined by assimp) 
     */
    static void fromRawValue(Set<AiSceneFlag> set, int rawValue) {
        
        for (AiSceneFlag type : AiSceneFlag.values()) {
            if ((type.m_rawValue & rawValue) != 0) {
                set.add(type);
            }
        }
    }
    
    
    /**
     * Constructor.
     * 
     * @param rawValue maps java enum to c/c++ integer enum values
     */
    private AiSceneFlag(int rawValue) {
        m_rawValue = rawValue;
    }    
}
