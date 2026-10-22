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
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * Simple 4x4 matrix of floats.
 */
public final class AiMatrix4f {
    /**
     * Wraps the given array of floats as matrix.
     * <p>
     * 
     * The array must have exactly 16 entries. The data in the array must be in
     * row-major order.
     * 
     * @param data
     *            the array to wrap, may not be null
     */
    public AiMatrix4f(float[] data) {
        if (data == null) {
            throw new IllegalArgumentException("data may not be null");
        }
        if (data.length != 16) {
            throw new IllegalArgumentException("array length is not 16");
        }

        m_data = data;
    }

    /**
     * Gets an element of the matrix.
     * 
     * @param row
     *            the row
     * @param col
     *            the column
     * @return the element at the given position
     */
    public float get(int row, int col) {
        if (row < 0 || row > 3) {
            throw new IndexOutOfBoundsException("Index: " + row + ", Size: 4");
        }
        if (col < 0 || col > 3) {
            throw new IndexOutOfBoundsException("Index: " + col + ", Size: 4");
        }

        return m_data[row * 4 + col];
    }

    /**
     * Stores the matrix in a new direct ByteBuffer with native byte order.
     * <p>
     * 
     * The returned buffer can be passed to rendering APIs such as LWJGL, e.g.,
     * as parameter for <code>GL20.glUniformMatrix4()</code>. Be sure to set
     * <code>transpose</code> to <code>true</code> in this case, as OpenGL
     * expects the matrix in column order.
     * 
     * @return a new native order, direct ByteBuffer
     */
    public FloatBuffer toByteBuffer() {
        ByteBuffer bbuf = ByteBuffer.allocateDirect(16 * 4);
        bbuf.order(ByteOrder.nativeOrder());
        FloatBuffer fbuf = bbuf.asFloatBuffer();
        fbuf.put(m_data);
        fbuf.flip();

        return fbuf;
    }

    
    @Override
    public String toString() {
        StringBuilder buf = new StringBuilder();

        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                buf.append(m_data[row * 4 + col]).append(" ");
            }
            buf.append("\n");
        }

        return buf.toString();
    }


    /**
     * Data buffer.
     */
    private final float[] m_data;
}
