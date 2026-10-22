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
 * Defines how UV coordinates outside the [0...1] range are handled.<p>
 *
 * Commonly referred to as 'wrapping mode'.
 */
public enum AiTextureMapMode {
    /** 
     * A texture coordinate u|v is translated to u%1|v%1. 
     */
    WRAP(0x0),

    
    /** 
     * Texture coordinates outside [0...1] are clamped to the nearest 
     * valid value.
     */
    CLAMP(0x1),


    /** 
     * A texture coordinate u|v becomes u%1|v%1 if (u-(u%1))%2 is zero and
     * 1-(u%1)|1-(v%1) otherwise.
     */
    MIRROR(0x2),
    
    
    /** 
     * If the texture coordinates for a pixel are outside [0...1] the texture 
     * is not applied to that pixel.
     */
    DECAL(0x3);

    
    /**
     * Utility method for converting from c/c++ based integer enums to java 
     * enums.<p>
     * 
     * This method is intended to be used from JNI and my change based on
     * implementation needs.
     * 
     * @param rawValue an integer based enum value (as defined by assimp) 
     * @return the enum value corresponding to rawValue
     */
    static AiTextureMapMode fromRawValue(int rawValue) {
        for (AiTextureMapMode type : AiTextureMapMode.values()) {
            if (type.m_rawValue == rawValue) {
                return type;
            }
        }
        
        throw new IllegalArgumentException("unexptected raw value: " + 
                rawValue);
    }
    
    
    /**
     * Constructor.
     * 
     * @param rawValue maps java enum to c/c++ integer enum values
     */
    private AiTextureMapMode(int rawValue) {
        m_rawValue = rawValue;
    }
    
    
    /**
     * The mapped c/c++ integer enum value.
     */
    private final int m_rawValue;
}
