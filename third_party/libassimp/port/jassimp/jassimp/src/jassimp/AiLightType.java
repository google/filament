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
 * List of light types supported by {@link AiLight}.
 */
public enum AiLightType {
    /**
     * A directional light source.<p>
     * 
     * A directional light has a well-defined direction but is infinitely far 
     * away. That's quite a good approximation for sun light.
     */
    DIRECTIONAL(0x1),

    
    /**
     * A point light source.<p>
     * 
     * A point light has a well-defined position in space but no direction - 
     * it emits light in all directions. A normal bulb is a point light.
     */
    POINT(0x2),

    
    /**
     * A spot light source.<p>
     * 
     * A spot light emits light in a specific angle. It has a position and a 
     * direction it is pointing to. A good example for a spot light is a light
     * spot in sport arenas.
     */
    SPOT(0x3),


    /**
     *  The generic light level of the world, including the bounces of all other
     *  lightsources. <p>
     *
     *  Typically, there's at most one ambient light in a scene.
     *  This light type doesn't have a valid position, direction, or
     *  other properties, just a color.
     */
    AMBIENT(0x4);


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
    static AiLightType fromRawValue(int rawValue) {
        for (AiLightType type : AiLightType.values()) {
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
    private AiLightType(int rawValue) {
        m_rawValue = rawValue;
    }
    
    
    /**
     * The mapped c/c++ integer enum value.
     */
    private final int m_rawValue;
}
