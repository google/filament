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
import java.util.ArrayList;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;
import java.util.Set;


/**
 * Data structure for a material.<p>
 *
 * Depending on the imported scene and scene format, individual properties
 * might be present or not. A list of all imported properties can be retrieved
 * via {@link #getProperties()}.<p>
 * 
 * This class offers <code>getXXX()</code> for all supported properties. These 
 * methods are fail-save, i.e., will return a default value when the 
 * corresponding property is not set. To change the built in default values, 
 * use the <code>setDefaultXXX()</code> methods.<p>
 * 
 * If your application expects a certain set of properties to be available,
 * the {@link #hasProperties(Set)} method can be used to check whether all
 * these properties are actually set. If this check fails, you can still
 * use this material via the <code>getXXX()</code> methods without special 
 * error handling code as the implementation guarantees to return default
 * values for missing properties. This check will not work on texture related
 * properties (i.e., properties starting with <code>TEX_</code>).
 */
public final class AiMaterial {
    /**
     * Enumerates all supported material properties.
     */
    public static enum PropertyKey {
        /**
         * Name.
         */
        NAME("?mat.name", String.class),
        
        /**
         * Two-sided flag.
         */
        TWO_SIDED("$mat.twosided", Integer.class),
        
        /**
         * Shading mode.
         */
        SHADING_MODE("$mat.shadingm", AiShadingMode.class),
        
        /**
         * Wireframe flag.
         */
        WIREFRAME("$mat.wireframe", Integer.class),
        
        /**
         * Blend mode.
         */
        BLEND_MODE("$mat.blend", AiBlendMode.class),
        
        /**
         * Opacity.
         */
        OPACITY("$mat.opacity", Float.class),
        
        /**
         * Bump scaling.
         */
        BUMP_SCALING("$mat.bumpscaling", Float.class),
        
        
        /**
         * Shininess.
         */
        SHININESS("$mat.shininess", Float.class),
        
        
        /**
         * Reflectivity.
         */
        REFLECTIVITY("$mat.reflectivity", Float.class),
        
        
        /**
         * Shininess strength.
         */
        SHININESS_STRENGTH("$mat.shinpercent", Float.class),
        
        
        /**
         * Refract index.
         */
        REFRACTI("$mat.refracti", Float.class),
        
        
        /**
         * Diffuse color.
         */
        COLOR_DIFFUSE("$clr.diffuse", Object.class),
        
        
        /**
         * Ambient color.
         */
        COLOR_AMBIENT("$clr.ambient", Object.class),
        
        
        /**
         * Ambient color.
         */
        COLOR_SPECULAR("$clr.specular", Object.class),
        
        
        /**
         * Emissive color.
         */
        COLOR_EMISSIVE("$clr.emissive", Object.class),
        
        
        /**
         * Transparent color.
         */
        COLOR_TRANSPARENT("$clr.transparent", Object.class),
        
        
        /**
         * Reflective color.
         */
        COLOR_REFLECTIVE("$clr.reflective", Object.class),
        
        
        /**
         * Global background image.
         */
        GLOBAL_BACKGROUND_IMAGE("?bg.global", String.class),
        
        
        /**
         * Texture file path.
         */
        TEX_FILE("$tex.file", String.class),
        
        
        /**
         * Texture uv index.
         */
        TEX_UV_INDEX("$tex.uvwsrc", Integer.class),
        
        
        /**
         * Texture blend factor.
         */
        TEX_BLEND("$tex.blend", Float.class),
        
        
        /**
         * Texture operation.
         */
        TEX_OP("$tex.op", AiTextureOp.class),
        
        
        /**
         * Texture map mode for u axis.
         */
        TEX_MAP_MODE_U("$tex.mapmodeu", AiTextureMapMode.class),
        
        
        /**
         * Texture map mode for v axis.
         */
        TEX_MAP_MODE_V("$tex.mapmodev", AiTextureMapMode.class),
        
        
        /**
         * Texture map mode for w axis.
         */
        TEX_MAP_MODE_W("$tex.mapmodew", AiTextureMapMode.class);
        
        /**
         * Constructor.
         * 
         * @param key key name as used by assimp
         * @param type key type, used for casts and checks
         */
        private PropertyKey(String key, Class<?> type) {
            m_key = key;
            m_type = type;
        }
        
        
        /**
         * Key.
         */
        private final String m_key;
        
        
        /**
         * Type.
         */
        private final Class<?> m_type;
    }
    
    
    /**
     *  A very primitive RTTI system for the contents of material properties.
     */
    public static enum PropertyType {
        /** 
         * Array of single-precision (32 Bit) floats.
         */
        FLOAT(0x1),


        /** 
         * The material property is a string.
         */
        STRING(0x3),


        /** 
         * Array of (32 Bit) integers.
         */
        INTEGER(0x4),


        /** 
         * Simple binary buffer, content undefined. Not convertible to anything.
         */
        BUFFER(0x5);


        /**
         * Utility method for converting from c/c++ based integer enums to java 
         * enums.<p>
         * 
         * This method is intended to be used from JNI and my change based on
         * implementation needs.
         * 
         * @param rawValue an integer based enum value (as defined by assimp) 
         * @return the enum value corresponding to rawValue
         */
        static PropertyType fromRawValue(int rawValue) {
            for (PropertyType type : PropertyType.values()) {
                if (type.m_rawValue == rawValue) {
                    return type;
                }
            }

            throw new IllegalArgumentException("unexptected raw value: " + 
                    rawValue);
        }


        /**
         * Constructor.
         * 
         * @param rawValue maps java enum to c/c++ integer enum values
         */
        private PropertyType(int rawValue) {
            m_rawValue = rawValue;
        }


        /**
         * The mapped c/c++ integer enum value.
         */
        private final int m_rawValue;
    }
    

    /**
     * Data structure for a single material property.<p>
     *
     * As an user, you'll probably never need to deal with this data structure.
     * Just use the provided get() family of functions to query material 
     * properties easily. 
     */
    public static final class Property {
        /**
         * Constructor.
         * 
         * @param key
         * @param semantic
         * @param index
         * @param type
         * @param data
         */
        Property(String key, int semantic, int index, int type, 
                Object data) {
            
            m_key = key;
            m_semantic = semantic;
            m_index = index;
            m_type = PropertyType.fromRawValue(type);
            m_data = data;
        }
        
        
        /**
         * Constructor.
         * 
         * @param key
         * @param semantic
         * @param index
         * @param type
         * @param dataLen
         */
        Property(String key, int semantic, int index, int type,
                int dataLen) {
            
            m_key = key;
            m_semantic = semantic;
            m_index = index;
            m_type = PropertyType.fromRawValue(type);
            
            ByteBuffer b = ByteBuffer.allocateDirect(dataLen);
            b.order(ByteOrder.nativeOrder());
            
            m_data = b;
        }
        
        
        /** 
         * Returns the key of the property.<p>
         * 
         * Keys are generally case insensitive. 
         * 
         * @return the key
         */
        public String getKey() {
            return m_key;
        }
        

        /** 
         * Textures: Specifies their exact usage semantic.
         * For non-texture properties, this member is always 0 
         * (or, better-said, #aiTextureType_NONE).
         * 
         * @return the semantic
         */
        public int getSemantic() {
            return m_semantic;
        }

        
        /** 
         * Textures: Specifies the index of the texture.
         * For non-texture properties, this member is always 0.
         * 
         * @return the index
         */
        public int getIndex() {
            return m_index;
        }
        

        /** 
         * Type information for the property.<p>
         *
         * Defines the data layout inside the data buffer. This is used
         * by the library internally to perform debug checks and to 
         * utilize proper type conversions. 
         * (It's probably a hacky solution, but it works.)
         * 
         * @return the type
         */
        public PropertyType getType() {
            return m_type;
        }
        

        /** 
         * Binary buffer to hold the property's value.
         * The size of the buffer is always mDataLength.
         * 
         * @return the data
         */
        Object getData() {
            return m_data;
        }
        
        
        /**
         * Key.
         */
        private final String m_key;
        
        
        /**
         * Semantic.
         */
        private final int m_semantic;
        
        
        /**
         * Index.
         */
        private final int m_index;
        
        
        /**
         * Type.
         */
        private final PropertyType m_type;
        
        
        /**
         * Data.
         */
        private final Object m_data;
    }
    
    
    /**
     * Constructor.
     */
    AiMaterial() {
        /* nothing to do */
    }
    
    
    /**
     * Checks whether the given set of properties is available.
     * 
     * @param keys the keys to check
     * @return true if all properties are available, false otherwise
     */
    public boolean hasProperties(Set<PropertyKey> keys) {
        for (PropertyKey key : keys) {
            if (null == getProperty(key.m_key)) {
                return false;
            }
        }
        
        return true;
    }
    
    
    /**
     * Sets a default value.<p>
     * 
     * The passed in Object must match the type of the key as returned by
     * the corresponding <code>getXXX()</code> method.
     * 
     * @param key the key
     * @param defaultValue the new default, may not be null
     * @throws IllegalArgumentException if defaultValue is null or has a wrong
     *              type
     */
    public void setDefault(PropertyKey key, Object defaultValue) {
        if (null == defaultValue) {
            throw new IllegalArgumentException("defaultValue may not be null");
        }
        if (key.m_type != defaultValue.getClass()) {
            throw new IllegalArgumentException(
                    "defaultValue has wrong type, " +
                    "expected: " + key.m_type + ", found: " + 
                    defaultValue.getClass());
        }
        
        m_defaults.put(key, defaultValue);
    }
    

    // {{ Fail-save Getters
    /**
     * Returns the name of the material.<p>
     * 
     * If missing, defaults to empty string
     * 
     * @return the name
     */
    public String getName() {
        return getTyped(PropertyKey.NAME, String.class);
    }
    
    
    /**
     * Returns the two-sided flag.<p>
     * 
     * If missing, defaults to 0
     * 
     * @return the two-sided flag
     */
    public int getTwoSided() {
        return getTyped(PropertyKey.TWO_SIDED, Integer.class);
    }
    
    
    /**
     * Returns the shading mode.<p>
     * 
     * If missing, defaults to {@link AiShadingMode#FLAT}
     * 
     * @return the shading mode
     */
    public AiShadingMode getShadingMode() {
        Property p = getProperty(PropertyKey.SHADING_MODE.m_key);
        
        if (null == p || null == p.getData()) {
            return (AiShadingMode) m_defaults.get(PropertyKey.SHADING_MODE);
        }
        
        return AiShadingMode.fromRawValue((Integer) p.getData());
    }
    
    
    /**
     * Returns the wireframe flag.<p>
     * 
     * If missing, defaults to 0
     * 
     * @return the wireframe flag
     */
    public int getWireframe() {
        return getTyped(PropertyKey.WIREFRAME, Integer.class);
    }
    
    
    /**
     * Returns the blend mode.<p>
     * 
     * If missing, defaults to {@link AiBlendMode#DEFAULT}
     * 
     * @return the blend mode
     */
    public AiBlendMode getBlendMode() {
        Property p = getProperty(PropertyKey.BLEND_MODE.m_key);
        
        if (null == p || null == p.getData()) {
            return (AiBlendMode) m_defaults.get(PropertyKey.BLEND_MODE);
        }
        
        return AiBlendMode.fromRawValue((Integer) p.getData());
    }
    
    
    /**
     * Returns the opacity.<p>
     * 
     * If missing, defaults to 1.0
     * 
     * @return the opacity
     */
    public float getOpacity() {
        return getTyped(PropertyKey.OPACITY, Float.class);
    }
    
    
    /**
     * Returns the bump scaling factor.<p>
     * 
     * If missing, defaults to 1.0
     * 
     * @return the bump scaling factor
     */
    public float getBumpScaling() {
        return getTyped(PropertyKey.BUMP_SCALING, Float.class);
    }
    
    
    /**
     * Returns the shininess.<p>
     * 
     * If missing, defaults to 1.0
     * 
     * @return the shininess
     */
    public float getShininess() {
        return getTyped(PropertyKey.SHININESS, Float.class);
    }
    
    
    /**
     * Returns the reflectivity.<p>
     * 
     * If missing, defaults to 0.0
     * 
     * @return the reflectivity
     */
    public float getReflectivity() {
        return getTyped(PropertyKey.REFLECTIVITY, Float.class);
    }
    
    
    /**
     * Returns the shininess strength.<p>
     * 
     * If missing, defaults to 0.0
     * 
     * @return the shininess strength
     */
    public float getShininessStrength() {
        return getTyped(PropertyKey.SHININESS_STRENGTH, Float.class);
    }
    
    
    /**
     * Returns the refract index.<p>
     * 
     * If missing, defaults to 0.0
     * 
     * @return the refract index
     */
    public float getRefractIndex() {
        return getTyped(PropertyKey.REFRACTI, Float.class);
    }
    
    
    /**
     * Returns the diffuse color.<p>
     * 
     * If missing, defaults to opaque white (1.0, 1.0, 1.0, 1.0)<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the diffuse color
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getDiffuseColor(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        Property p = getProperty(PropertyKey.COLOR_DIFFUSE.m_key);
        
        if (null == p || null == p.getData()) {
            Object def = m_defaults.get(PropertyKey.COLOR_DIFFUSE);
            if (def == null) {
                return (C) Jassimp.wrapColor4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            
            return (C) def;
        }
        
        return (C) p.getData();
    }
    
    
    /**
     * Returns the ambient color.<p>
     * 
     * If missing, defaults to opaque white (1.0, 1.0, 1.0, 1.0)<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the ambient color
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getAmbientColor(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        Property p = getProperty(PropertyKey.COLOR_AMBIENT.m_key);
        
        if (null == p || null == p.getData()) {
            Object def = m_defaults.get(PropertyKey.COLOR_AMBIENT);
            if (def == null) {
                return (C) Jassimp.wrapColor4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            
            return (C) def;
        }
        
        return (C) p.getData();
    }
    
    
    /**
     * Returns the specular color.<p>
     * 
     * If missing, defaults to opaque white (1.0, 1.0, 1.0, 1.0)<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the specular color
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getSpecularColor(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        Property p = getProperty(PropertyKey.COLOR_SPECULAR.m_key);
        
        if (null == p || null == p.getData()) {
            Object def = m_defaults.get(PropertyKey.COLOR_SPECULAR);
            if (def == null) {
                return (C) Jassimp.wrapColor4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            
            return (C) def;
        }
        
        return (C) p.getData();
    }
    
    
    /**
     * Returns the emissive color.<p>
     * 
     * If missing, defaults to opaque white (1.0, 1.0, 1.0, 1.0)<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the emissive color
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getEmissiveColor(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        Property p = getProperty(PropertyKey.COLOR_EMISSIVE.m_key);
        
        if (null == p || null == p.getData()) {
            Object def = m_defaults.get(PropertyKey.COLOR_EMISSIVE);
            if (def == null) {
                return (C) Jassimp.wrapColor4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            
            return (C) def;
        }
        
        return (C) p.getData();
    }
    
    
    /**
     * Returns the transparent color.<p>
     * 
     * If missing, defaults to opaque white (1.0, 1.0, 1.0, 1.0)<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the transparent color
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getTransparentColor(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        Property p = getProperty(PropertyKey.COLOR_TRANSPARENT.m_key);
        
        if (null == p || null == p.getData()) {
            Object def = m_defaults.get(PropertyKey.COLOR_TRANSPARENT);
            if (def == null) {
                return (C) Jassimp.wrapColor4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            
            return (C) def;
        }
        
        return (C) p.getData();
    }
    
    
    /**
     * Returns the reflective color.<p>
     * 
     * If missing, defaults to opaque white (1.0, 1.0, 1.0, 1.0)<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the reflective color
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> C getReflectiveColor(
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        Property p = getProperty(PropertyKey.COLOR_REFLECTIVE.m_key);
        
        if (null == p || null == p.getData()) {
            Object def = m_defaults.get(PropertyKey.COLOR_REFLECTIVE);
            if (def == null) {
                return (C) Jassimp.wrapColor4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            
            return (C) def;
        }
        
        return (C) p.getData();
    }
    
    
    /**
     * Returns the global background image.<p>
     * 
     * If missing, defaults to empty string
     * 
     * @return the global background image
     */
    public String getGlobalBackgroundImage() {
        return getTyped(PropertyKey.GLOBAL_BACKGROUND_IMAGE, String.class);
    }
    
    
    /**
     * Returns the number of textures of the given type.
     * 
     * @param type the type
     * @return the number of textures
     */
    public int getNumTextures(AiTextureType type) {
        return m_numTextures.get(type);
    }
    
    
    /**
     * Returns the texture file.<p>
     * 
     * If missing, defaults to empty string
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the file
     * @throws IndexOutOfBoundsException if index is invalid
     */
    public String getTextureFile(AiTextureType type, int index) {
        checkTexRange(type, index);
        
        return getTyped(PropertyKey.TEX_FILE, type, index, String.class);
    }
    
    
    /**
     * Returns the index of the UV coordinate set used by the texture.<p>
     * 
     * If missing, defaults to 0
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the UV index
     * @throws IndexOutOfBoundsException if index is invalid
     */
    public int getTextureUVIndex(AiTextureType type, int index) {
        checkTexRange(type, index);
        
        return getTyped(PropertyKey.TEX_UV_INDEX, type, index, Integer.class);
    }
    
    
    /**
     * Returns the blend factor of the texture.<p>
     * 
     * If missing, defaults to 1.0
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the blend factor
     */
    public float getBlendFactor(AiTextureType type, int index) {
        checkTexRange(type, index);
        
        return getTyped(PropertyKey.TEX_BLEND, type, index, Float.class);
    }
    
    
    /**
     * Returns the texture operation.<p>
     * 
     * If missing, defaults to {@link AiTextureOp#ADD}
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the texture operation
     */
    public AiTextureOp getTextureOp(AiTextureType type, int index) {
        checkTexRange(type, index);
        
        Property p = getProperty(PropertyKey.TEX_OP.m_key);
        
        if (null == p || null == p.getData()) {
            return (AiTextureOp) m_defaults.get(PropertyKey.TEX_OP);
        }
        
        return AiTextureOp.fromRawValue((Integer) p.getData());
    }
    
    
    /**
     * Returns the texture mapping mode for the u axis.<p>
     * 
     * If missing, defaults to {@link AiTextureMapMode#CLAMP}
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the texture mapping mode
     */
    public AiTextureMapMode getTextureMapModeU(AiTextureType type, int index) {
        checkTexRange(type, index);
        
        Property p = getProperty(PropertyKey.TEX_MAP_MODE_U.m_key);
        
        if (null == p || null == p.getData()) {
            return (AiTextureMapMode) m_defaults.get(
                    PropertyKey.TEX_MAP_MODE_U);
        }
        
        return AiTextureMapMode.fromRawValue((Integer) p.getData());
    }
    
    
    /**
     * Returns the texture mapping mode for the v axis.<p>
     * 
     * If missing, defaults to {@link AiTextureMapMode#CLAMP}
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the texture mapping mode
     */
    public AiTextureMapMode getTextureMapModeV(AiTextureType type, int index) {
        checkTexRange(type, index);
        
        Property p = getProperty(PropertyKey.TEX_MAP_MODE_V.m_key);
        
        if (null == p || null == p.getData()) {
            return (AiTextureMapMode) m_defaults.get(
                    PropertyKey.TEX_MAP_MODE_V);
        }
        
        return AiTextureMapMode.fromRawValue((Integer) p.getData());
    }
    
    
    /**
     * Returns the texture mapping mode for the w axis.<p>
     * 
     * If missing, defaults to {@link AiTextureMapMode#CLAMP}
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the texture mapping mode
     */
    public AiTextureMapMode getTextureMapModeW(AiTextureType type, int index) {
        checkTexRange(type, index);
        
        Property p = getProperty(PropertyKey.TEX_MAP_MODE_W.m_key);
        
        if (null == p || null == p.getData()) {
            return (AiTextureMapMode) m_defaults.get(
                    PropertyKey.TEX_MAP_MODE_W);
        }
        
        return AiTextureMapMode.fromRawValue((Integer) p.getData());
    }
    
    
    /**
     * Returns all information related to a single texture.
     * 
     * @param type the texture type
     * @param index the index in the texture stack
     * @return the texture information
     */
    public AiTextureInfo getTextureInfo(AiTextureType type, int index) {
        return new AiTextureInfo(type, index, getTextureFile(type, index), 
                getTextureUVIndex(type, index), getBlendFactor(type, index), 
                getTextureOp(type, index), getTextureMapModeW(type, index), 
                getTextureMapModeW(type, index), 
                getTextureMapModeW(type, index));
    }
    // }}
    
    // {{ Generic Getters
    /**
     * Returns a single property based on its key.
     * 
     * @param key the key
     * @return the property or null if the property is not set
     */
    public Property getProperty(String key) {
        for (Property property : m_properties) {
            if (property.getKey().equals(key)) {
                return property;
            }
        }
        
        return null;
    }
    
    
    /**
     * Returns a single property based on its key.
     * 
     * @param key the key
     * @param semantic the semantic type (texture type)
     * @param index the index
     * @return the property or null if the property is not set
     */
    public Property getProperty(String key, int semantic, int index) {
        for (Property property : m_properties) {
            if (property.getKey().equals(key) && 
                    property.m_semantic == semantic && 
                    property.m_index == index) {
                
                return property;
            }
        }
        
        return null;
    }
 
    
    /**
     * Returns all properties of the material.
     * 
     * @return the list of properties
     */
    public List<Property> getProperties() {
        return m_properties;
    }
    // }}
    
    
    /**
     * Helper method. Returns typed property data.
     * 
     * @param <T> type
     * @param key the key
     * @param clazz type
     * @return the data
     */
    private <T> T getTyped(PropertyKey key, Class<T> clazz) {
        Property p = getProperty(key.m_key);
        
        if (null == p || null == p.getData()) {
            return clazz.cast(m_defaults.get(key));
        }
        
        return clazz.cast(p.getData());
    }
    
    
    /**
     * Helper method. Returns typed property data.
     * 
     * @param <T> type
     * @param key the key
     * @param type the texture type
     * @param index the texture index
     * @param clazz type
     * @return the data
     */
    private <T> T getTyped(PropertyKey key, AiTextureType type, int index, 
            Class<T> clazz) {
        
        Property p = getProperty(key.m_key, AiTextureType.toRawValue(type), 
                index);
        
        if (null == p || null == p.getData()) {
            return clazz.cast(m_defaults.get(key));
        }
        
        return clazz.cast(p.getData());
    }

    
    /**
     * Checks that index is valid an throw an exception if not.
     * 
     * @param type the type
     * @param index the index to check
     */
    private void checkTexRange(AiTextureType type, int index) {
        if (index < 0 || index > m_numTextures.get(type)) {
            throw new IndexOutOfBoundsException("Index: " + index + ", Size: " +
                    m_numTextures.get(type));
        }
    }
    
    
    /**
     * Defaults for missing properties.
     */
    private Map<PropertyKey, Object> m_defaults = 
            new EnumMap<PropertyKey, Object>(PropertyKey.class);
    
    {
        setDefault(PropertyKey.NAME,                    "");
        setDefault(PropertyKey.TWO_SIDED,               0);
        setDefault(PropertyKey.SHADING_MODE,            AiShadingMode.FLAT);
        setDefault(PropertyKey.WIREFRAME,               0);
        setDefault(PropertyKey.BLEND_MODE,              AiBlendMode.DEFAULT);
        setDefault(PropertyKey.OPACITY,                 1.0f);
        setDefault(PropertyKey.BUMP_SCALING,            1.0f);
        setDefault(PropertyKey.SHININESS,               1.0f);
        setDefault(PropertyKey.REFLECTIVITY,            0.0f);
        setDefault(PropertyKey.SHININESS_STRENGTH,      0.0f);
        setDefault(PropertyKey.REFRACTI,                0.0f);
        
        /* bypass null checks for colors */
        m_defaults.put(PropertyKey.COLOR_DIFFUSE,       null);
        m_defaults.put(PropertyKey.COLOR_AMBIENT,       null);
        m_defaults.put(PropertyKey.COLOR_SPECULAR,      null);
        m_defaults.put(PropertyKey.COLOR_EMISSIVE,      null);
        m_defaults.put(PropertyKey.COLOR_TRANSPARENT,   null);
        m_defaults.put(PropertyKey.COLOR_REFLECTIVE,    null);
        
        setDefault(PropertyKey.GLOBAL_BACKGROUND_IMAGE, "");
        
        /* texture related values */
        setDefault(PropertyKey.TEX_FILE,                "");
        setDefault(PropertyKey.TEX_UV_INDEX,            0);
        setDefault(PropertyKey.TEX_BLEND,               1.0f);
        setDefault(PropertyKey.TEX_OP,                  AiTextureOp.ADD);
        setDefault(PropertyKey.TEX_MAP_MODE_U,          AiTextureMapMode.CLAMP);
        setDefault(PropertyKey.TEX_MAP_MODE_V,          AiTextureMapMode.CLAMP);
        setDefault(PropertyKey.TEX_MAP_MODE_W,          AiTextureMapMode.CLAMP);
        
        /* ensure we have defaults for everything */
        for (PropertyKey key : PropertyKey.values()) {
            if (!m_defaults.containsKey(key)) {
                throw new IllegalStateException("missing default for: " + key);
            }
        }
    }
    
    
    /**
     * This method is used by JNI, do not call or modify.
     * 
     * @param type the type
     * @param number the number
     */
    @SuppressWarnings("unused")
    private void setTextureNumber(int type, int number) {
        m_numTextures.put(AiTextureType.fromRawValue(type), number);
    }
    
    
    /**
     * List of properties.
     */
    private final List<Property> m_properties = new ArrayList<Property>();
    
    
    /**
     * Number of textures for each type.
     */
    private final Map<AiTextureType, Integer> m_numTextures = 
            new EnumMap<AiTextureType, Integer>(AiTextureType.class);
}
