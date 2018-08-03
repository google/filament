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

import java.util.ArrayList;
import java.util.List;


/**
 * A single bone of a mesh.<p>
 *
 * A bone has a name by which it can be found in the frame hierarchy and by
 * which it can be addressed by animations. In addition it has a number of 
 * influences on vertices.<p>
 * 
 * This class is designed to be mutable, i.e., the returned collections are
 * writable and may be modified.
 */
public final class AiBone {
    /**
     * Constructor.
     */
    AiBone() {
        /* nothing to do */
    }
    
    
    /**
     * Returns the name of the bone.
     * 
     * @return the name
     */
    public String getName() {
        return m_name;
    }
    
    
    /**
     * Returns the number of bone weights.<p>
     * 
     * This method exists for compatibility with the native assimp API.
     * The returned value is identical to <code>getBoneWeights().size()</code>
     * 
     * @return the number of weights
     */
    public int getNumWeights() {
        return m_boneWeights.size();
    }
    
    
    /**
     * Returns a list of bone weights.
     * 
     * @return the bone weights
     */
    public List<AiBoneWeight> getBoneWeights() {
        return m_boneWeights;
    }
    
    
    /**
     * Returns the offset matrix.<p>
     * 
     * The offset matrix is a 4x4 matrix that transforms from mesh space to 
     * bone space in bind pose.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * 
     * @return the offset matrix
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> M4 getOffsetMatrix(
            AiWrapperProvider<V3, M4, C, N, Q>  wrapperProvider) {
        
        return (M4) m_offsetMatrix;
    }
    
    
    /**
     * Name of the bone.
     */
    private String m_name;
    
    
    /**
     * Bone weights.
     */
    private final List<AiBoneWeight> m_boneWeights = 
            new ArrayList<AiBoneWeight>();
    
    
    /**
     * Offset matrix.
     */
    private Object m_offsetMatrix; 
}
