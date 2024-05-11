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
 * Wrapper for 3-dimensional vectors.<p>
 * 
 * This wrapper is also used to represent 1- and 2-dimensional vectors. In 
 * these cases only the x (or the x and y coordinate) will be used.
 * Accessing unused components will throw UnsupportedOperationExceptions.<p>
 * 
 * The wrapper is writable, i.e., changes performed via the set-methods will
 * modify the underlying mesh.
 */
public final class AiVector {
    /**
     * Constructor.
     * 
     * @param buffer the buffer to wrap
     * @param offset offset into buffer
     * @param numComponents number vector of components
     */
    public AiVector(ByteBuffer buffer, int offset, int numComponents) {
        if (null == buffer) {
            throw new IllegalArgumentException("buffer may not be null");
        }
        
        m_buffer = buffer;
        m_offset = offset;
        m_numComponents = numComponents;
    }
    
    
    /**
     * Returns the x value.
     * 
     * @return the x value
     */
    public float getX() {
        return m_buffer.getFloat(m_offset);
    }
    
    
    /**
     * Returns the y value.<p>
     * 
     * May only be called on 2- or 3-dimensional vectors.
     * 
     * @return the y value
     */
    public float getY() {
        if (m_numComponents <= 1) {
            throw new UnsupportedOperationException(
                    "vector has only 1 component");
        }
        
        return m_buffer.getFloat(m_offset + 4);
    }
    
    
    /**
     * Returns the z value.<p>
     * 
     * May only be called on 3-dimensional vectors.
     * 
     * @return the z value
     */
    public float getZ() {
        if (m_numComponents <= 2) {
            throw new UnsupportedOperationException(
                    "vector has only 2 components");
        }
        
        return m_buffer.getFloat(m_offset + 8);
    }
    
    
    /**
     * Sets the x component.
     * 
     * @param x the new value
     */
    public void setX(float x) {
        m_buffer.putFloat(m_offset, x);
    }
    
    
    /**
     * Sets the y component.<p>
     * 
     * May only be called on 2- or 3-dimensional vectors.
     * 
     * @param y the new value
     */
    public void setY(float y) {
        if (m_numComponents <= 1) {
            throw new UnsupportedOperationException(
                    "vector has only 1 component");
        }
        
        m_buffer.putFloat(m_offset + 4, y);
    }
    
    
    /**
     * Sets the z component.<p>
     * 
     * May only be called on 3-dimensional vectors.
     * 
     * @param z the new value
     */
    public void setZ(float z) {
        if (m_numComponents <= 2) {
            throw new UnsupportedOperationException(
                    "vector has only 2 components");
        }
        
        m_buffer.putFloat(m_offset + 8, z);
    }
    
    
    /**
     * Returns the number of components in this vector.
     * 
     * @return the number of components
     */
    public int getNumComponents() {
        return m_numComponents;
    }
    
    
    @Override
    public String toString() {
        return "[" + getX() + ", " + getY() + ", " + getZ() + "]";
    }
    
    
    /**
     * Wrapped buffer.
     */
    private final ByteBuffer m_buffer;
    
    
    /**
     * Offset into m_buffer.
     */
    private final int m_offset;
    
    
    /**
     * Number of components. 
     */
    private final int m_numComponents;
}
