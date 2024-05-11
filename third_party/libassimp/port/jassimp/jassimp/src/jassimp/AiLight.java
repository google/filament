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
 * Describes a light source.<p>
 *
 * Assimp supports multiple sorts of light sources, including
 * directional, point and spot lights. All of them are defined with just
 * a single structure and distinguished by their parameters.
 * Note - some file formats (such as 3DS, ASE) export a "target point" -
 * the point a spot light is looking at (it can even be animated). Assimp
 * writes the target point as a subnode of a spotlights's main node,
 * called "&lt;spotName&gt;.Target". However, this is just additional 
 * information then, the transformation tracks of the main node make the
 * spot light already point in the right direction.
 */
public final class AiLight {
    /**
     * Constructor.
     * 
     * @param name
     * @param type
     * @param position
     * @param direction
     * @param attenuationConstant
     * @param attenuationLinear
     * @param attenuationQuadratic
     * @param diffuse
     * @param specular
     * @param ambient
     * @param innerCone
     * @param outerCone
     */
    AiLight(String name, int type, Object position, Object direction, 
            float attenuationConstant, float attenuationLinear, 
            float attenuationQuadratic, Object diffuse, Object specular, 
            Object ambient, float innerCone, float outerCone) {

        m_name = name;
        m_type = AiLightType.fromRawValue(type);
        m_position = position;
        m_direction = direction;
        m_attenuationConstant = attenuationConstant;
        m_attenuationLinear = attenuationLinear;
        m_attenuationQuadratic = attenuationQuadratic;
        m_diffuse = diffuse;
        m_specular = specular;
        m_ambient = ambient;
        m_innerCone = innerCone;
        m_outerCone = outerCone;
    }


    /**
     * Returns the name of the light source.<p>
     *
     * There must be a node in the scenegraph with the same name.
     * This node specifies the position of the light in the scene
     * hierarchy and can be animated.
     * 
     * @return the name
     */
    public String getName() {
        return m_name;
    }
    
    
    /**
     * Returns The type of the light source.
     * 
     * @return the type
     */
    public AiLightType getType() {
        return m_type;
    }
    
    
    /**
     * Returns the position of the light.<p>
     * 
     * The position is relative to the transformation of the scene graph node 
     * corresponding to the light. The position is undefined for directional 
     * lights.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiVector}.
     * 
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * 
     * @return the position
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> V3 getPosition(AiWrapperProvider<V3, M4, C, N, Q> 
            wrapperProvider) {
        
        return (V3) m_position;
    }
    
    
    /**
     * Returns the direction of the light.<p>
     * 
     * The direction is relative to the transformation of the scene graph node 
     * corresponding to the light. The direction is undefined for point lights.
     * The vector may be normalized, but it needn't..<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the position
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> V3 getDirection(AiWrapperProvider<V3, M4, C, N, Q> 
            wrapperProvider) {
        
        return (V3) m_direction;
    }
    
    
    /** 
     * Constant light attenuation factor.<p> 
     *
     * The intensity of the light source at a given distance 'd' from
     * the light's position is 
     * <code>Atten = 1/( att0 + att1 * d + att2 * d*d)</code> 
     * This member corresponds to the att0 variable in the equation.
     * Naturally undefined for directional lights.
     *  
     * @return the constant light attenuation factor
     */
    public float getAttenuationConstant() {
        return m_attenuationConstant;
    }


    /** 
     * Linear light attenuation factor.<p> 
     *
     * The intensity of the light source at a given distance 'd' from
     * the light's position is
     * <code>Atten = 1/( att0 + att1 * d + att2 * d*d)</code>
     * This member corresponds to the att1 variable in the equation.
     * Naturally undefined for directional lights.
     * 
     * @return the linear light attenuation factor
     */
    public float getAttenuationLinear() {
        return m_attenuationLinear;
    }


    /** 
     * Quadratic light attenuation factor.<p> 
     *  
     * The intensity of the light source at a given distance 'd' from
     * the light's position is
     * <code>Atten = 1/( att0 + att1 * d + att2 * d*d)</code>
     * This member corresponds to the att2 variable in the equation.
     * Naturally undefined for directional lights.
     * 
     * @return the quadratic light attenuation factor
     */
    public float getAttenuationQuadratic() {
        return m_attenuationQuadratic;
    }


    /** 
     * Diffuse color of the light source.<p>
     *
     * The diffuse light color is multiplied with the diffuse 
     * material color to obtain the final color that contributes
     * to the diffuse shading term.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiColor}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the diffuse color (alpha will be 1)
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getColorDiffuse(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        return (C) m_diffuse;
    }

    
    /** 
     * Specular color of the light source.<p>
     *
     * The specular light color is multiplied with the specular
     * material color to obtain the final color that contributes
     * to the specular shading term.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiColor}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the specular color (alpha will be 1)
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getColorSpecular(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        return (C) m_specular;
    }

    
    /** 
     * Ambient color of the light source.<p>
     *
     * The ambient light color is multiplied with the ambient
     * material color to obtain the final color that contributes
     * to the ambient shading term. Most renderers will ignore
     * this value it, is just a remaining of the fixed-function pipeline
     * that is still supported by quite many file formats.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiColor}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the ambient color (alpha will be 1)
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getColorAmbient(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        return (C) m_ambient;
    }
    

    /** 
     * Inner angle of a spot light's light cone.<p>
     *
     * The spot light has maximum influence on objects inside this
     * angle. The angle is given in radians. It is 2PI for point 
     * lights and undefined for directional lights.
     * 
     * @return the inner angle
     */
    public float getAngleInnerCone() {
        return m_innerCone;
    }
    

    /** 
     * Outer angle of a spot light's light cone.<p>
     *
     * The spot light does not affect objects outside this angle.
     * The angle is given in radians. It is 2PI for point lights and 
     * undefined for directional lights. The outer angle must be
     * greater than or equal to the inner angle.
     * It is assumed that the application uses a smooth
     * interpolation between the inner and the outer cone of the
     * spot light. 
     * 
     * @return the outer angle
     */
    public float getAngleOuterCone() {
        return m_outerCone;
    }


    /**
     * Name.
     */
    private final String m_name;


    /**
     * Type.
     */
    private final AiLightType m_type;


    /**
     * Position.
     */
    private final Object m_position;


    /**
     * Direction.
     */
    private final Object m_direction;


    /**
     * Constant attenuation.
     */
    private final float m_attenuationConstant;


    /**
     * Linear attenuation. 
     */
    private final float m_attenuationLinear;


    /**
     * Quadratic attenuation.
     */
    private final float m_attenuationQuadratic;
    
    
    /**
     * Diffuse color.
     */
    private final Object m_diffuse;
    
    
    /**
     * Specular color.
     */
    private final Object m_specular;
    
    
    /**
     * Ambient color.
     */
    private final Object m_ambient;
    
    
    /**
     * Inner cone of spotlight.
     */
    private final float m_innerCone;
    
    
    /**
     * Outer cone of spotlight.
     */
    private final float m_outerCone;
}
