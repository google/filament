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
 * The root structure of the imported data.<p>
 * 
 * Everything that was imported from the given file can be accessed from here.
 * <p>
 * Jassimp copies all data into "java memory" during import and frees 
 * resources allocated by native code after scene loading is completed. No
 * special care has to be taken for freeing resources, unreferenced jassimp 
 * objects (including the scene itself) are eligible to garbage collection like
 * any other java object.  
 */
public final class AiScene {
    /**
     * Constructor.
     */
    AiScene() {
        /* nothing to do */
    }
    
    
    /**
     * Returns the number of meshes contained in the scene.<p>
     * 
     * This method is provided for completeness reasons. It will return the 
     * same value as <code>getMeshes().size()</code>
     * 
     * @return the number of meshes
     */
    public int getNumMeshes() {
        return m_meshes.size();
    }
    

    /**
     * Returns the meshes contained in the scene.<p>
     * 
     * If there are no meshes in the scene, an empty collection is returned
     * 
     * @return the list of meshes
     */
    public List<AiMesh> getMeshes() {
        return m_meshes;
    }
    
    
    /** 
     * Returns the number of materials in the scene.<p>
     * 
     * This method is provided for completeness reasons. It will return the 
     * same value as <code>getMaterials().size()</code>
     * 
     * @return the number of materials
     */
    public int getNumMaterials() {
        return m_materials.size();
    }

    
    /** 
     * Returns the list of materials.<p>
     * 
     * Use the index given in each aiMesh structure to access this
     * array. If the {@link AiSceneFlag#INCOMPLETE} flag is not set there will
     * always be at least ONE material.
     * 
     * @return the list of materials
     */
    public List<AiMaterial> getMaterials() {
        return m_materials;
    }
    
    
    /** 
     * Returns the number of animations in the scene.<p>
     * 
     * This method is provided for completeness reasons. It will return the 
     * same value as <code>getAnimations().size()</code>
     * 
     * @return the number of materials
     */
    public int getNumAnimations() {
        return m_animations.size();
    }

    
    /** 
     * Returns the list of animations.
     * 
     * @return the list of animations
     */
    public List<AiAnimation> getAnimations() {
        return m_animations;
    }


    /** 
     * Returns the number of light sources in the scene.<p>
     * 
     * This method is provided for completeness reasons. It will return the 
     * same value as <code>getLights().size()</code>
     * 
     * @return the number of lights
     */
    public int getNumLights() {
        return m_lights.size();
    }
     

    /** 
     * Returns the list of light sources.<p>
     * 
     * Light sources are fully optional, the returned list may be empty
     * 
     * @return a possibly empty list of lights
     */
    public List<AiLight> getLights() {
        return m_lights; 
    }
    
    
    /**
     * Returns the number of cameras in the scene.<p>
     * 
     * This method is provided for completeness reasons. It will return the
     * same value as <code>getCameras().size()</code>
     * 
     * @return the number of cameras
     */
    public int getNumCameras() {
        return m_cameras.size();
    }
    
    
    /**
     * Returns the list of cameras.<p>
     * 
     * Cameras are fully optional, the returned list may be empty
     * 
     * @return a possibly empty list of cameras
     */
    public List<AiCamera> getCameras() {
        return m_cameras;
    }

    
    /**
     * Returns the scene graph root.
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the scene graph root
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> N getSceneRoot(AiWrapperProvider<V3, M4, C, N, Q> 
            wrapperProvider) {

        return (N) m_sceneRoot;
    } 


    @Override
    public String toString() {
        return "AiScene (" + m_meshes.size() + " mesh/es)";
    }


    /**
     * Meshes.
     */
    private final List<AiMesh> m_meshes = new ArrayList<AiMesh>();
    
    
    /**
     * Materials.
     */
    private final List<AiMaterial> m_materials = new ArrayList<AiMaterial>();
    
    
    /**
     * Animations.
     */
    private final List<AiAnimation> m_animations = new ArrayList<AiAnimation>();
    
    
    /**
     * Lights.
     */
    private final List<AiLight> m_lights = new ArrayList<AiLight>();
    
    
    /**
     * Cameras.
     */
    private final List<AiCamera> m_cameras = new ArrayList<AiCamera>();
    
    
    /**
     * Scene graph root.
     */
    private Object m_sceneRoot;
}
