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
 * Defines how the Nth texture of a specific type is combined with the result 
 * of all previous layers.<p>
 *
 * Example (left: key, right: value): <br>
 * <code><pre>
 *  DiffColor0     - gray
 *  DiffTextureOp0 - aiTextureOpMultiply
 *  DiffTexture0   - tex1.png
 *  DiffTextureOp0 - aiTextureOpAdd
 *  DiffTexture1   - tex2.png
 * </pre></code>
 *  
 * Written as equation, the final diffuse term for a specific pixel would be: 
 * <code><pre>
 *  diffFinal = DiffColor0 * sampleTex(DiffTexture0,UV0) + 
 *     sampleTex(DiffTexture1,UV0) * diffContrib;
 * </pre></code>
 * where 'diffContrib' is the intensity of the incoming light for that pixel.
 */
public enum AiTextureOp {
    /** 
     * <code>T = T1 * T2</code>.
     */
    MULTIPLY(0x0),

    
    /** 
     * <code>T = T1 + T2</code>.
      */
    ADD(0x1),

    
    /** 
     * <code>T = T1 - T2</code>.
      */
    SUBTRACT(0x2),

    
    /** 
     * <code>T = T1 / T2</code>.
     */
    DIVIDE(0x3),

    
    /** 
     * <code>T = (T1 + T2) - (T1 * T2)</code> .
     */
    SMOOTH_ADD(0x4),

    
    /** 
     * <code>T = T1 + (T2-0.5)</code>.
     */
    SIGNED_ADD(0x5);
    
    
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
    static AiTextureOp fromRawValue(int rawValue) {
        for (AiTextureOp type : AiTextureOp.values()) {
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
    private AiTextureOp(int rawValue) {
        m_rawValue = rawValue;
    }
    
    
    /**
     * The mapped c/c++ integer enum value.
     */
    private final int m_rawValue;
}
