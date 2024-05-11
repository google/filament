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
 * Wrapper provider using jassimp built in types.
 */
public final class AiBuiltInWrapperProvider implements AiWrapperProvider<
        AiVector, AiMatrix4f, AiColor, AiNode, AiQuaternion> {

    @Override
    public AiVector wrapVector3f(ByteBuffer buffer, int offset, 
            int numComponents) {
        
        return new AiVector(buffer, offset, numComponents);
    }


    @Override
    public AiMatrix4f wrapMatrix4f(float[] data) {
        return new AiMatrix4f(data);
    }

    
    @Override
    public AiColor wrapColor(ByteBuffer buffer, int offset) {
        return new AiColor(buffer, offset);
    }


    @Override
    public AiNode wrapSceneNode(Object parent, Object matrix,
            int[] meshReferences, String name) {
        
        return new AiNode((AiNode) parent, matrix, meshReferences, name);
    }


    @Override
    public AiQuaternion wrapQuaternion(ByteBuffer buffer, int offset) {
        return new AiQuaternion(buffer, offset);
    }
}
