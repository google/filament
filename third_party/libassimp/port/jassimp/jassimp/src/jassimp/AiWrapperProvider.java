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
 * Provides wrapper objects for raw data buffers.<p>
 * 
 * It is likely that applications using Jassimp will already have a scene
 * graph implementation and/ or the typical math related classes such as
 * vectors, matrices, etc.<p>
 * 
 * To ease the integration with existing code, Jassimp can be customized to 
 * represent the scene graph and compound data structures such as vectors and 
 * matrices with user supplied classes.<p>
 * 
 * All methods returning wrapped objects rely on the AiWrapperProvider to 
 * create individual instances. Custom wrappers can be created by implementing
 * AiWrapperProvider and registering the implementation via 
 * {@link Jassimp#setWrapperProvider(AiWrapperProvider)} <b>before</b> the
 * scene is imported.<p>
 * 
 * The methods returning wrapped types take an AiWrapperProvider instance. This
 * instance must match the instance set via
 * {@link Jassimp#setWrapperProvider(AiWrapperProvider)}. The method parameter
 * is used to infer the type of the returned object. The passed in wrapper 
 * provider is not necessarily used to actually create the wrapped object, as 
 * the object may be cached for performance reasons. <b>It is not possible to 
 * use different AiWrapperProviders throughout the lifetime of an imported
 * scene.</b>
 * 
 * @param <V3> the type used to represent vectors
 * @param <M4> the type used to represent matrices
 * @param <C> the type used to represent colors
 * @param <N> the type used to represent scene graph nodes
 * @param <Q> the type used to represent quaternions
 */
public interface AiWrapperProvider<V3, M4, C, N, Q> {
    /**
     * Wraps a vector.<p>
     * 
     * Most vectors are 3-dimensional, i.e., with 3 components. The exception
     * are texture coordinates, which may be 1- or 2-dimensional. A vector
     * consists of numComponents floats (x,y,z) starting from offset
     * 
     * @param buffer the buffer to wrap
     * @param offset the offset into buffer
     * @param numComponents the number of components
     * @return the wrapped vector
     */
    V3 wrapVector3f(ByteBuffer buffer, int offset, int numComponents);
    
    
    /**
     * Wraps a 4x4 matrix of floats.<p>
     * 
     * The calling code will allocate a new array for each invocation of this 
     * method. It is safe to store a reference to the  passed in array and
     * use the array to store the matrix data. 
     * 
     * @param data the matrix data in row-major order
     * @return the wrapped matrix
     */
    M4 wrapMatrix4f(float[] data);
    
    
    /**
     * Wraps a RGBA color.<p>
     * 
     * A color consists of 4 float values (r,g,b,a) starting from offset
     * 
     * @param buffer the buffer to wrap
     * @param offset the offset into buffer
     * @return the wrapped color
     */
    C wrapColor(ByteBuffer buffer, int offset);
    
    
    /**
     * Wraps a scene graph node.<p>
     * 
     * See {@link AiNode} for a description of the scene graph structure used 
     * by assimp.<p>
     * 
     * The parent node is either null or an instance returned by this method.
     * It is therefore safe to cast the passed in parent object to the 
     * implementation specific type
     * 
     * @param parent the parent node
     * @param matrix the transformation matrix
     * @param meshReferences array of mesh references (indexes)
     * @param name the name of the node
     * @return the wrapped scene graph node
     */
    N wrapSceneNode(Object parent, Object matrix, int[] meshReferences,
            String name);
    
    
    /**
     * Wraps a quaternion.<p>
     * 
     * A quaternion consists of 4 float values (w,x,y,z) starting from offset
     * 
     * @param buffer the buffer to wrap
     * @param offset the offset into buffer
     * @return the wrapped quaternion
     */
    Q wrapQuaternion(ByteBuffer buffer, int offset);
}
