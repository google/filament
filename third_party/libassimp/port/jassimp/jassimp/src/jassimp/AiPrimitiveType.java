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
 * Enumerates the types of geometric primitives supported by Assimp.<p>
 */
public enum AiPrimitiveType {
    /**
     * A point primitive.
     */
    POINT(0x1),
    
    
    /**
     * A line primitive.
     */
    LINE(0x2),
    
    
    /**
     * A triangular primitive.
     */
    TRIANGLE(0x4),
    
    
    /**
     * A higher-level polygon with more than 3 edges.<p>
     * 
     * A triangle is a polygon, but polygon in this context means
     * "all polygons that are not triangles". The "Triangulate"-Step is provided
     * for your convenience, it splits all polygons in triangles (which are much
     * easier to handle).
     */
    POLYGON(0x8);

    
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
    static void fromRawValue(Set<AiPrimitiveType> set, int rawValue) {
        
        for (AiPrimitiveType type : AiPrimitiveType.values()) {
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
    private AiPrimitiveType(int rawValue) {
        m_rawValue = rawValue;
    }
    
    
    /**
     * The mapped c/c++ integer enum value.
     */
    private final int m_rawValue;
}
