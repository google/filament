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

import java.nio.ByteBuffer;


/**
 * Wrapper for a quaternion.<p>
 * 
 * The wrapper is writable, i.e., changes performed via the set-methods will
 * modify the underlying mesh/animation.
 */
public final class AiQuaternion {
    /**
     * Constructor.
     * 
     * @param buffer the buffer to wrap
     * @param offset offset into buffer
     */
    public AiQuaternion(ByteBuffer buffer, int offset) {
        if (null == buffer) {
            throw new IllegalArgumentException("buffer may not be null");
        }
        
        m_buffer = buffer;
        m_offset = offset;
    }
    
    
    /**
     * Returns the x value.
     * 
     * @return the x value
     */
    public float getX() {
        return m_buffer.getFloat(m_offset + 4);
    }
    
    
    /**
     * Returns the y value.
     * 
     * @return the y value
     */
    public float getY() {
        return m_buffer.getFloat(m_offset + 8);
    }
    
    
    /**
     * Returns the z value.
     * 
     * @return the z value
     */
    public float getZ() {
        return m_buffer.getFloat(m_offset + 12);
    }
    
    
    /**
     * Returns the w value.
     * 
     * @return the w value
     */
    public float getW() {
        return m_buffer.getFloat(m_offset);
    }
    
    
    /**
     * Sets the x component.
     * 
     * @param x the new value
     */
    public void setX(float x) {
        m_buffer.putFloat(m_offset + 4, x);
    }
    
    
    /**
     * Sets the y component.
     * 
     * @param y the new value
     */
    public void setY(float y) {
        m_buffer.putFloat(m_offset + 8, y);
    }
    
    
    /**
     * Sets the z component.
     * 
     * @param z the new value
     */
    public void setZ(float z) {
        m_buffer.putFloat(m_offset + 12, z);
    }
    
    
    /**
     * Sets the z component.
     * 
     * @param w the new value
     */
    public void setW(float w) {
        m_buffer.putFloat(m_offset, w);
    }
    
    
    @Override
    public String toString() {
        return "[" + getX() + ", " + getY() + ", " + getZ() + ", " + 
                getW() + "]";
    }
    
    
    /**
     * Wrapped buffer.
     */
    private final ByteBuffer m_buffer;
    
    
    /**
     * Offset into m_buffer.
     */
    private final int m_offset;
}