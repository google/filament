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
 * Defines how an animation channel behaves outside the defined time range.
 */
public enum AiAnimBehavior {
    /** 
     * The value from the default node transformation is taken.
     */
    DEFAULT(0x0),  

    
    /** 
     * The nearest key value is used without interpolation. 
     */
    CONSTANT(0x1),

    
    /** 
     * The value of the nearest two keys is linearly extrapolated for the 
     * current time value.
     */
    LINEAR(0x2),

    
    /** 
     * The animation is repeated.<p>
     *
     * If the animation key go from n to m and the current time is t, use the 
     * value at (t-n) % (|m-n|).
     */
    REPEAT(0x3);
    
    
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
    static AiAnimBehavior fromRawValue(int rawValue) {
        for (AiAnimBehavior type : AiAnimBehavior.values()) {
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
    private AiAnimBehavior(int rawValue) {
        m_rawValue = rawValue;
    }


    /**
     * The mapped c/c++ integer enum value.
     */
    private final int m_rawValue;
}
