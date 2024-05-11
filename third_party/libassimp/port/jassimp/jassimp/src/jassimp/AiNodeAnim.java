/*
---------------------------------------------------------------------------
Open Asset Import Library - Java Binding (jassimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2015, assimp team

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


/** 
 * Describes the animation of a single node.<p>
 * 
 * The node name ({@link #getNodeName()} specifies the bone/node which is 
 * affected by this animation channel. The keyframes are given in three 
 * separate series of values, one each for position, rotation and scaling. 
 * The transformation matrix computed from these values replaces the node's 
 * original transformation matrix at a specific time.<p>
 * 
 * This means all keys are absolute and not relative to the bone default pose.
 * The order in which the transformations are applied is - as usual - 
 * scaling, rotation, translation.<p>
 *
 * <b>Note:</b> All keys are returned in their correct, chronological order.
 * Duplicate keys don't pass the validation step. Most likely there
 * will be no negative time values, but they are not forbidden also (so 
 * implementations need to cope with them!)<p>
 * 
 * Like {@link AiMesh}, the animation related classes offer a Buffer API, a 
 * Direct API and a wrapped API. Please consult the documentation of 
 * {@link AiMesh} for a description and comparison of these APIs.
 */
public final class AiNodeAnim {
    /**
     * Size of one position key entry.
     */
    private final int POS_KEY_SIZE = Jassimp.NATIVE_AIVEKTORKEY_SIZE;
    
    /**
     * Size of one rotation key entry.
     */
    private final int ROT_KEY_SIZE = Jassimp.NATIVE_AIQUATKEY_SIZE;
    
    /**
     * Size of one scaling key entry.
     */
    private final int SCALE_KEY_SIZE = Jassimp.NATIVE_AIVEKTORKEY_SIZE;
    
    
    /**
     * Constructor.
     * 
     * @param nodeName name of corresponding scene graph node
     * @param numPosKeys number of position keys
     * @param numRotKeys number of rotation keys
     * @param numScaleKeys number of scaling keys
     * @param preBehavior behavior before animation start
     * @param postBehavior behavior after animation end
     */
    AiNodeAnim(String nodeName, int numPosKeys, int numRotKeys, 
            int numScaleKeys, int preBehavior, int postBehavior) {
        
        m_nodeName = nodeName;
        m_numPosKeys = numPosKeys;
        m_numRotKeys = numRotKeys;
        m_numScaleKeys = numScaleKeys;
        m_preState = AiAnimBehavior.fromRawValue(preBehavior);
        m_postState = AiAnimBehavior.fromRawValue(postBehavior);
        
        m_posKeys = ByteBuffer.allocateDirect(numPosKeys * POS_KEY_SIZE);
        m_posKeys.order(ByteOrder.nativeOrder());
        
        m_rotKeys = ByteBuffer.allocateDirect(numRotKeys * ROT_KEY_SIZE);
        m_rotKeys.order(ByteOrder.nativeOrder());
        
        m_scaleKeys = ByteBuffer.allocateDirect(numScaleKeys * SCALE_KEY_SIZE);
        m_scaleKeys.order(ByteOrder.nativeOrder());
    }
    
    
    /** 
     * Returns the name of the scene graph node affected by this animation.<p>
     * 
     * The node must exist and it must be unique.
     * 
     * @return the name of the affected node
     */
    public String getNodeName() {
        return m_nodeName;
    }
    

    /** 
     * Returns the number of position keys.
     * 
     * @return the number of position keys
     */
    public int getNumPosKeys() {
        return m_numPosKeys;
    }
    

    /** 
     * Returns the buffer with position keys of this animation channel.<p>
     * 
     * Position keys consist of a time value (double) and a position (3D vector
     * of floats), resulting in a total of 20 bytes per entry. 
     * The buffer contains {@link #getNumPosKeys()} of these entries.<p>
     *
     * If there are position keys, there will also be at least one
     * scaling and one rotation key.<p>
     * 
     * @return a native order, direct ByteBuffer
     */
    public ByteBuffer getPosKeyBuffer() {
        ByteBuffer buf = m_posKeys.duplicate();
        buf.order(ByteOrder.nativeOrder());
        
        return buf;
    }
    
    
    /**
     * Returns the time component of the specified position key. 
     * 
     * @param keyIndex the index of the position key
     * @return the time component
     */
    public double getPosKeyTime(int keyIndex) {
        return m_posKeys.getDouble(POS_KEY_SIZE * keyIndex);
    }
    
    
    /**
     * Returns the position x component of the specified position key. 
     * 
     * @param keyIndex the index of the position key
     * @return the x component
     */
    public float getPosKeyX(int keyIndex) {
        return m_posKeys.getFloat(POS_KEY_SIZE * keyIndex + 8);
    }
    
    
    /**
     * Returns the position y component of the specified position key. 
     * 
     * @param keyIndex the index of the position key
     * @return the y component
     */
    public float getPosKeyY(int keyIndex) {
        return m_posKeys.getFloat(POS_KEY_SIZE * keyIndex + 12);
    }

    
    /**
     * Returns the position z component of the specified position key. 
     * 
     * @param keyIndex the index of the position key
     * @return the z component
     */
    public float getPosKeyZ(int keyIndex) {
        return m_posKeys.getFloat(POS_KEY_SIZE * keyIndex + 16);
    }
    
    
    /**
     * Returns the position as vector.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * 
     * @return the position as vector
     */
    public <V3, M4, C, N, Q> V3 getPosKeyVector(int keyIndex, 
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {

        return wrapperProvider.wrapVector3f(m_posKeys, 
                POS_KEY_SIZE * keyIndex + 8, 3);
    }
    

    /** 
     * Returns the number of rotation keys.
     * 
     * @return the number of rotation keys
     */
    public int getNumRotKeys() {
       return m_numRotKeys; 
    }

    
    /** 
     * Returns the buffer with rotation keys of this animation channel.<p> 
     * 
     * Rotation keys consist of a time value (double) and a quaternion (4D 
     * vector of floats), resulting in a total of 24 bytes per entry. The 
     * buffer contains {@link #getNumRotKeys()} of these entries.<p>
     *
     * If there are rotation keys, there will also be at least one
     * scaling and one position key.
     * 
     * @return a native order, direct ByteBuffer
     */
    public ByteBuffer getRotKeyBuffer() {
        ByteBuffer buf = m_rotKeys.duplicate();
        buf.order(ByteOrder.nativeOrder());
        
        return buf;
    }
    
    
    /**
     * Returns the time component of the specified rotation key. 
     * 
     * @param keyIndex the index of the position key
     * @return the time component
     */
    public double getRotKeyTime(int keyIndex) {
        return m_rotKeys.getDouble(ROT_KEY_SIZE * keyIndex);
    }
    
    
    /**
     * Returns the rotation w component of the specified rotation key. 
     * 
     * @param keyIndex the index of the position key
     * @return the w component
     */
    public float getRotKeyW(int keyIndex) {
        return m_rotKeys.getFloat(ROT_KEY_SIZE * keyIndex + 8);
    }
    
    
    /**
     * Returns the rotation x component of the specified rotation key. 
     * 
     * @param keyIndex the index of the position key
     * @return the x component
     */
    public float getRotKeyX(int keyIndex) {
        return m_rotKeys.getFloat(ROT_KEY_SIZE * keyIndex + 12);
    }
    
    
    /**
     * Returns the rotation y component of the specified rotation key. 
     * 
     * @param keyIndex the index of the position key
     * @return the y component
     */
    public float getRotKeyY(int keyIndex) {
        return m_rotKeys.getFloat(ROT_KEY_SIZE * keyIndex + 16);
    }

    
    /**
     * Returns the rotation z component of the specified rotation key. 
     * 
     * @param keyIndex the index of the position key
     * @return the z component
     */
    public float getRotKeyZ(int keyIndex) {
        return m_rotKeys.getFloat(ROT_KEY_SIZE * keyIndex + 20);
    }
    
    
    /**
     * Returns the rotation as quaternion.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiQuaternion}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * 
     * @return the rotation as quaternion
     */
    public <V3, M4, C, N, Q> Q getRotKeyQuaternion(int keyIndex, 
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {

        return wrapperProvider.wrapQuaternion(m_rotKeys, 
                ROT_KEY_SIZE * keyIndex + 8);
    } 
    
    
    /** 
     * Returns the number of scaling keys.
     * 
     * @return the number of scaling keys
     */
    public int getNumScaleKeys() {
        return m_numScaleKeys;
    }
    

    /** 
     * Returns the buffer with scaling keys of this animation channel.<p>
     * 
     * Scaling keys consist of a time value (double) and a 3D vector of floats,
     * resulting in a total of 20 bytes per entry. The buffer 
     * contains {@link #getNumScaleKeys()} of these entries.<p>
     * 
     * If there are scaling keys, there will also be at least one
     * position and one rotation key.
     * 
     * @return a native order, direct ByteBuffer
     */
    public ByteBuffer getScaleKeyBuffer() {
        ByteBuffer buf = m_scaleKeys.duplicate();
        buf.order(ByteOrder.nativeOrder());
        
        return buf;
    }

    
    /**
     * Returns the time component of the specified scaling key. 
     * 
     * @param keyIndex the index of the position key
     * @return the time component
     */
    public double getScaleKeyTime(int keyIndex) {
        return m_scaleKeys.getDouble(SCALE_KEY_SIZE * keyIndex);
    }
    
    
    /**
     * Returns the scaling x component of the specified scaling key. 
     * 
     * @param keyIndex the index of the position key
     * @return the x component
     */
    public float getScaleKeyX(int keyIndex) {
        return m_scaleKeys.getFloat(SCALE_KEY_SIZE * keyIndex + 8);
    }
    
    
    /**
     * Returns the scaling y component of the specified scaling key. 
     * 
     * @param keyIndex the index of the position key
     * @return the y component
     */
    public float getScaleKeyY(int keyIndex) {
        return m_scaleKeys.getFloat(SCALE_KEY_SIZE * keyIndex + 12);
    }

    
    /**
     * Returns the scaling z component of the specified scaling key. 
     * 
     * @param keyIndex the index of the position key
     * @return the z component
     */
    public float getScaleKeyZ(int keyIndex) {
        return m_scaleKeys.getFloat(SCALE_KEY_SIZE * keyIndex + 16);
    }
    
    
    /**
     * Returns the scaling factor as vector.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * 
     * @return the scaling factor as vector
     */
    public <V3, M4, C, N, Q> V3 getScaleKeyVector(int keyIndex, 
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {

        return wrapperProvider.wrapVector3f(m_scaleKeys, 
                SCALE_KEY_SIZE * keyIndex + 8, 3);
    }


    /** 
     * Defines how the animation behaves before the first key is encountered.
     * <p>
     *
     * The default value is {@link AiAnimBehavior#DEFAULT} (the original 
     * transformation matrix of the affected node is used).
     * 
     * @return the animation behavior before the first key
     */
    public AiAnimBehavior getPreState() {
        return m_preState;
    }
    

    /** 
     * Defines how the animation behaves after the last key was processed.<p>
     *
     * The default value is {@link AiAnimBehavior#DEFAULT} (the original
     * transformation matrix of the affected node is taken).
     * 
     * @return the animation behavior before after the last key
     */
    public AiAnimBehavior getPostState() {
        return m_postState;
    }
    
    
    /**
     * Node name.
     */
    private final String m_nodeName;
    
    
    /**
     * Number of position keys.
     */
    private final int m_numPosKeys;
    
    
    /**
     * Buffer with position keys.
     */
    private ByteBuffer m_posKeys;
    
    
    /**
     * Number of rotation keys.
     */
    private final int m_numRotKeys;
    
    
    /**
     * Buffer for rotation keys.
     */
    private ByteBuffer m_rotKeys;
    
    
    /**
     * Number of scaling keys.
     */
    private final int m_numScaleKeys;
    
    
    /**
     * Buffer for scaling keys.
     */
    private ByteBuffer m_scaleKeys;
    
    
    /**
     * Pre animation behavior.
     */
    private final AiAnimBehavior m_preState;
    
    
    /**
     * Post animation behavior.
     */
    private final AiAnimBehavior m_postState;
}
