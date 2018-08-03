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
 * Data structure for texture related material properties.
 */
public final class AiTextureInfo {
    
    /**
     * Constructor.
     * 
     * @param type type
     * @param index index
     * @param file file
     * @param uvIndex uv index
     * @param blend blend factor
     * @param texOp texture operation
     * @param mmU map mode for u axis
     * @param mmV map mode for v axis
     * @param mmW map mode for w axis
     */
    AiTextureInfo(AiTextureType type, int index, String file,
            int uvIndex, float blend, AiTextureOp texOp, AiTextureMapMode mmU,
            AiTextureMapMode mmV, AiTextureMapMode mmW) {
        
        m_type = type;
        m_index = index;
        m_file = file;
        m_uvIndex = uvIndex;
        m_blend = blend;
        m_textureOp = texOp;
        m_textureMapModeU = mmU;
        m_textureMapModeV = mmV;
        m_textureMapModeW = mmW;
    }
    

    /**
     * Specifies the type of the texture (e.g. diffuse, specular, ...).
     * 
     * @return the type.
     */
    public AiTextureType getType() {
        return m_type;
    }
    
    
    /**
     * Index of the texture in the texture stack.<p>
     * 
     * Each type maintains a stack of textures, i.e., there may be a diffuse.0,
     * a diffuse.1, etc
     * 
     * @return the index
     */
    public int getIndex() {
        return m_index;
    }
    
    
    /**
     * Returns the path to the texture file.
     * 
     * @return the path
     */
    public String getFile() {
        return m_file;
    }
    
    
    /**
     * Returns the index of the UV coordinate set.
     * 
     * @return the uv index
     */
    public int getUVIndex() {
        return m_uvIndex;
    }
    
    
    /**
     * Returns the blend factor.
     * 
     * @return the blend factor
     */
    public float getBlend() {
        return m_blend;
    }
    
    
    /**
     * Returns the texture operation used to combine this texture and the
     * preceding texture in the stack.
     * 
     * @return the texture operation
     */
    public AiTextureOp getTextureOp() {
        return m_textureOp;
    }
    
    
    /**
     * Returns the texture map mode for U texture axis.
     * 
     * @return the texture map mode
     */
    public AiTextureMapMode getTextureMapModeU() {
        return m_textureMapModeU;
    }
    
    
    /**
     * Returns the texture map mode for V texture axis.
     * 
     * @return the texture map mode
     */
    public AiTextureMapMode getTextureMapModeV() {
        return m_textureMapModeV;
    }
    
    
    /**
     * Returns the texture map mode for W texture axis.
     * 
     * @return the texture map mode
     */
    public AiTextureMapMode getTextureMapModeW() {
        return m_textureMapModeW;
    }
    
    
    /**
     * Type.
     */
    private final AiTextureType m_type;
    
    
    /**
     * Index.
     */
    private final int m_index;
    
    
    /**
     * Path.
     */
    private final String m_file;
    
    
    /**
     * UV index.
     */
    private final int m_uvIndex;
    
    
    /**
     * Blend factor.
     */
    private final float m_blend;
    
    
    /**
     * Texture operation.
     */
    private final AiTextureOp m_textureOp;
    
    
    /**
     * Map mode U axis.
     */
    private final AiTextureMapMode m_textureMapModeU;
    
    
    /**
     * Map mode V axis.
     */
    private final AiTextureMapMode m_textureMapModeV;
    
    
    /**
     * Map mode W axis.
     */
    private final AiTextureMapMode m_textureMapModeW;
}
