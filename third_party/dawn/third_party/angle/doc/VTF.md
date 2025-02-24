# Vertex Texture Fetch

This page details the steps necessary to implement vertex texture fetch in ANGLE
and documents some of the pitfalls that may be encountered along the way.

# Details

Tasks to implement vertex texture support.

1.  add/enable vertex shader texture look up functions in compiler & HLSL
    translator.
    *   add texture2DLod, texture2DProjLod (2 variants), textureCubeLod (these
        are **only** valid in vertex shaders)
    *   ensure other (non-bias/non-LOD) texture functions work in vertex shaders
    *   non-mipmapped textures use the only level available
    *   mipmapped textures use only the base level (ie level 0).
2.  update implementation-dependent constants in Context.h
    *   MAX\_VERTEX\_TEXTURE\_IMAGE\_UNITS = 4
    *   MAX\_COMBINED\_TEXTURE\_IMAGE\_UNITS =
        MAX\_VERTEX\_TEXTURE\_IMAGE\_UNITS + MAX\_TEXTURE\_IMAGE\_UNITS (ie 20).
    *   these limits have to change based on the d3d device characteristics. For
        example we likely don't want to advertise vertex image units on SM2.0
        cards (unless we end up using software vertex processing).
    *   detection of hardware support for various formats, types, etc.
    *   As a first pass, use the "hasVertexTextures" check that Aras suggested
        to only enable VTF on DX10 NVIDIA and AMD parts, and SM3 Intel parts.
    *   If this proves insufficient, there are other things we can do, but it
        involves using software vertex processing for unsupported formats and
        system memory copies of textures -- all stuff which is rather annoying
        and likely to hurt performance (see point 4. below).
3.  add support and handling for vertex textures/samplers in the API.
    *   any textures used in a vertex shader need to get assigned to the special
        samplers in d3d9
    *   there are only 4 of them (D3DVERTEXTEXTURESAMPLER0..
        D3DVERTEXTEXTURESAMPLER3)
    *   if a texture is used in both vertex & fragment it counts twice against
        the "MAX\_COMBINED" limit (validated in Program::validateSamplers)
    *   there are a number of places in our code where we have arrays of size,
        or iterate over, MAX\_TEXTURE\_IMAGE\_UNITS. These will need to be
        changed to operate on MAX\_COMBINED\_TEXTURE\_IMAGE\_UNITS instead. A
        (possibly incomplete & outdated) list of areas that need to be updated
        is as follows:
    *   Program.h - increase size of mSamplers
    *   Context.h - increase size of samplerTexture
    *   glActiveTexture needs accept values in the range
        0..MAX\_COMBINED\_TEXTURE\_IMAGE\_UNITS-1
    *   Context::~Context
    *   GetIntegerv (2D\_BINDING, CUBE\_BINDING)
    *   Context::applyTextures
    *   Context::detachTexture
    *   Program::getSamplerMapping
    *   Program::dirtyAllSamplers
    *   Program::applyUniform1iv
    *   Program::unlink
    *   Program::validateSamplers
4.  handling the nasty corner cases: texture formats, filtering and cube
    textures.
    *   OpenGL doesn't provide any restrictions on what formats and/or types of
        textures can used for vertex textures, or if filtering can be enabled,
        whereas D3D9 does.
    *   Reference Rasterizer / Software Vertex Processing: all formats & types
        supported (including filtering)
    *   ATI R500 (on Google Code) cards do not support VTF (even though they are
        SM 3.0)
    *   ATI R600 (on Google Code) (and later) and in theory the Intel 965+,
        claim to support all texture formats/types we care about and some with
        filtering
    *   NVIDIA cards fall into two camps:
    *   dx9 SM3 (6&7 series): only R32F & A32B32G32R32F supported for 2D and no
        filtering, CUBE or VOL texture support
    *   dx10 (8+ series): only float texture formats for 2D, CUBE & VOLUME. no
        filtering (according to caps)
        *   further info from Aras P. suggests that all formats are supported on
            DX10 hardware, but are just not advertised.
    *   unsure what they do on these cards under OpenGL. Need to do more
        testing, but suspect software fallback.
