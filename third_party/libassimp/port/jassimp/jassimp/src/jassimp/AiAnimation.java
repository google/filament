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

import java.util.ArrayList;
import java.util.List;

/** 
 * An animation.<p>
 * 
 * An animation consists of keyframe data for a number of nodes. For 
 * each node affected by the animation a separate series of data is given.<p>
 * 
 * Like {@link AiMesh}, the animation related classes offer a Buffer API, a 
 * Direct API and a wrapped API. Please consult the documentation of 
 * {@link AiMesh} for a description and comparison of these APIs.
 */
public final class AiAnimation {
    /**
     * Name.
     */
    private final String m_name;
    
    /**
     * Duration.
     */
    private final double m_duration;
    
    /**
     * Ticks per second.
     */
    private final double m_ticksPerSecond;
        
    /**
     * Bone animation channels.
     */
    private final List<AiNodeAnim> m_nodeAnims = new ArrayList<AiNodeAnim>();

    /**
     * Constructor.
     * 
     * @param name name
     * @param duration duration
     * @param ticksPerSecond ticks per second
     */
    AiAnimation(String name, double duration, double ticksPerSecond) {
        m_name = name;
        m_duration = duration;
        m_ticksPerSecond = ticksPerSecond;
    }
    
    
    /** 
     * Returns the name of the animation.<p>
     * 
     * If the modeling package this data was exported from does support only 
     * a single animation channel, this name is usually empty (length is zero).
     * 
     * @return the name
     */
    public String getName() {
        return m_name;
    }
    

    /** 
     * Returns the duration of the animation in ticks.
     * 
     * @return the duration
     */
    public double getDuration() {
        return m_duration;
    }

    
    /** 
     * Returns the ticks per second.<p>
     * 
     * 0 if not specified in the imported file
     * 
     * @return the number of ticks per second
     */
    public double getTicksPerSecond() {
        return m_ticksPerSecond;
    }
    

    /** 
     * Returns the number of bone animation channels.<p>
     * 
     * Each channel affects a single node. This method will return the same
     * value as <code>getChannels().size()</code>
     * 
     * @return the number of bone animation channels
     */
    public int getNumChannels() {
        return m_nodeAnims.size();
    }
    

    /** 
     * Returns the list of bone animation channels.<p>
     * 
     * Each channel affects a single node. The array is mNumChannels in size.
     * 
     * @return the list of bone animation channels
     */
    public List<AiNodeAnim> getChannels() {
        return m_nodeAnims;
    }


    /** 
     * Returns the number of mesh animation channels.<p>
     * 
     * Each channel affects a single mesh and defines vertex-based animation.
     * This method will return the same value as 
     * <code>getMeshChannels().size()</code>
     * 
     * @return the number of mesh animation channels
     */
    public int getNumMeshChannels() {
        throw new UnsupportedOperationException("not implemented yet");
    }

    
    /** 
     * Returns the list of mesh animation channels.<p>
     * 
     * Each channel affects a single mesh.
     * 
     * @return the list of mesh animation channels
     */
    public List<AiMeshAnim> getMeshChannels() {
        throw new UnsupportedOperationException("not implemented yet");
    }    
}