/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// ---------------
// Buffer Wrappers
// ---------------

// These wrappers make it easy for JavaScript clients to pass large swaths of data to Filament. They
// copy the contents of the given typed array into the WASM heap, then return a low-level buffer
// descriptor object. If the given array was taken from the WASM heap, then they create a temporary
// copy because the input pointer becomes invalidated after allocating heap memory for the buffer
// descriptor.

/// Buffer ::function:: Constructs a [BufferDescriptor] by copying a typed array into the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// ::retval:: [BufferDescriptor]
Filament.Buffer = function(typedarray) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);

    // The only reason we need to create a copy here is that emscripten might "grow" its entire heap
    // (i.e. destroy and recreate) during the allocation of the BufferDescriptor, which would cause
    // detachment if the source array happens to be view into the old emscripten heap.
    const ta = typedarray.slice();

    const bd = new Filament.driver$BufferDescriptor(ta.byteLength);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);

    // getBytes() returns a view into the emscripten heap, this just does a memcpy into it.
    bd.getBytes().set(uint8array);

    return bd;
};

/// PixelBuffer ::function:: Constructs a [PixelBufferDescriptor] by copying a typed array into \
/// the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// format ::argument:: [PixelDataFormat]
/// datatype ::argument:: [PixelDataType]
/// ::retval:: [PixelBufferDescriptor]
Filament.PixelBuffer = function(typedarray, format, datatype) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    const ta = typedarray.slice();
    const bd = new Filament.driver$PixelBufferDescriptor(ta.byteLength, format, datatype);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

/// CompressedPixelBuffer ::function:: Constructs a [PixelBufferDescriptor] for compressed texture
/// data by copying a typed array into the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// cdatatype ::argument:: [CompressedPixelDataType]
/// faceSize ::argument:: Number of bytes in each face (cubemaps only)
/// ::retval:: [PixelBufferDescriptor]
Filament.CompressedPixelBuffer = function(typedarray, cdatatype, faceSize) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    faceSize = faceSize || typedarray.byteLength;
    const ta = typedarray.slice();
    const bd = new Filament.driver$PixelBufferDescriptor(ta.byteLength, cdatatype, faceSize, true);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

Filament._loadFilamesh = function(engine, buffer, definstance, matinstances) {
    matinstances = matinstances || {};
    const registry = new Filament.MeshReader$MaterialRegistry();
    for (let key in matinstances) {
        registry.set(key, matinstances[key]);
    }
    if (definstance) {
        registry.set("DefaultMaterial", definstance);
    }
    const mesh = Filament.MeshReader.loadMeshFromBuffer(engine, buffer, registry);
    const keys = registry.keys();
    for (let i = 0; i < keys.size(); i++) {
        const key = keys.get(i);
        const minstance = registry.get(key);
        matinstances[key] = minstance;
    }
    return {
        "renderable": mesh.renderable(),
        "vertexBuffer": mesh.vertexBuffer(),
        "indexBuffer": mesh.indexBuffer(),
    }
}

// ------------------
// Geometry Utilities
// ------------------

/// IcoSphere ::class:: Utility class for constructing spheres (requires glMatrix).
///
/// The constructor takes an integer subdivision level, with 0 being an icosahedron.
///
/// Exposes three arrays as properties:
///
/// - `icosphere.vertices` Float32Array of XYZ coordinates.
/// - `icosphere.tangents` Uint16Array (interpreted as half-floats) encoding the surface orientation
/// as quaternions.
/// - `icosphere.triangles` Uint16Array with triangle indices.
///
Filament.IcoSphere = function(nsubdivs) {
    const X = .525731112119133606;
    const Z = .850650808352039932;
    const N = 0.;
    this.vertices = new Float32Array([
        -X, +N, +Z, +X, +N, +Z, -X, +N, -Z, +X, +N, -Z ,
        +N, +Z, +X, +N, +Z, -X, +N, -Z, +X, +N, -Z, -X ,
        +Z, +X, +N, -Z, +X, +N, +Z, -X, +N, -Z, -X, +N ,
    ]);
    this.triangles = new Uint16Array([
        1,   4, 0,  4,  9,  0, 4,   5, 9, 8, 5,   4 ,  1,  8,  4 ,
        1,  10, 8, 10,  3,  8, 8,   3, 5, 3, 2,   5 ,  3,  7,  2 ,
        3,  10, 7, 10,  6,  7, 6,  11, 7, 6, 0,  11 ,  6,  1,  0 ,
        10,   1, 6, 11,  0,  9, 2,  11, 9, 5, 2,   9 , 11,  2,  7 ,
    ]);

    nsubdivs = nsubdivs || 0;
    while (nsubdivs-- > 0) {
        this.subdivide();
    }

    const nverts = this.vertices.length / 3;

    // This is a unit sphere, so normals = positions.
    const normals = this.vertices;

    // Perform computations.
    const sob = new Filament.SurfaceOrientation$Builder();
    sob.vertexCount(nverts);
    sob.normals(normals, 0)
    const orientation = sob.build();

    // Copy the results out of the helper.
    this.tangents = orientation.getQuats(nverts);

    // Free up the surface orientation helper now that we're done with it.
    orientation.delete();
}

Filament.IcoSphere.prototype.subdivide = function() {
    const srctris = this.triangles;
    const srcverts = this.vertices;
    const nsrctris = srctris.length / 3;
    const ndsttris = nsrctris * 4;
    const nsrcverts = srcverts.length / 3;
    const ndstverts = nsrcverts + nsrctris * 3;
    const dsttris = new Uint16Array(ndsttris * 3);
    const dstverts = new Float32Array(ndstverts * 3);
    dstverts.set(srcverts);
    let srcind = 0, dstind = 0, i3 = nsrcverts * 3, i4 = i3 + 3, i5 = i4 + 3;
    for (let tri = 0; tri < nsrctris; tri++, i3 += 9, i4 += 9, i5 += 9) {
        const i0 = srctris[srcind++] * 3;
        const i1 = srctris[srcind++] * 3;
        const i2 = srctris[srcind++] * 3;
        const v0 = srcverts.subarray(i0, i0 + 3);
        const v1 = srcverts.subarray(i1, i1 + 3);
        const v2 = srcverts.subarray(i2, i2 + 3);
        const v3 = dstverts.subarray(i3, i3 + 3);
        const v4 = dstverts.subarray(i4, i4 + 3);
        const v5 = dstverts.subarray(i5, i5 + 3);
        vec3.normalize(v3, vec3.add(v3, v0, v1));
        vec3.normalize(v4, vec3.add(v4, v1, v2));
        vec3.normalize(v5, vec3.add(v5, v2, v0));
        dsttris[dstind++] = i0 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i1 / 3;
        dsttris[dstind++] = i4 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i4 / 3;
        dsttris[dstind++] = i2 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i4 / 3;
    }
    this.triangles = dsttris;
    this.vertices = dstverts;
}

// ---------------
// Math Extensions
// ---------------

function clamp(v, least, most) {
    return Math.max(Math.min(most, v), least);
}

/// packSnorm16 ::function:: Converts a float in [-1, +1] into a half-float.
/// value ::argument:: float
/// ::retval:: half-float
Filament.packSnorm16 = function(value) {
    return Math.round(clamp(value, -1.0, 1.0) * 32767.0);
}

/// loadMathExtensions ::function:: Extends the [glMatrix](http://glmatrix.net/) math library.
/// Filament does not require its clients to use glMatrix, but if its usage is detected then
/// the [init] function will automatically call `loadMathExtensions`.
/// This defines the following functions:
/// - **vec4.packSnorm16** can be used to create half-floats (see [packSnorm16])
/// - **mat3.fromRotation** now takes an arbitrary axis
Filament.loadMathExtensions = function() {
    vec4.packSnorm16 = function(out, src) {
        out[0] = Filament.packSnorm16(src[0]);
        out[1] = Filament.packSnorm16(src[1]);
        out[2] = Filament.packSnorm16(src[2]);
        out[3] = Filament.packSnorm16(src[3]);
        return out;
    }
    // In gl-matrix, mat3 rotation assumes rotation about the Z axis, so here we add a function
    // to allow an arbitrary axis.
    const fromRotationZ = mat3.fromRotation;
    mat3.fromRotation = function(out, radians, axis) {
        if (axis) {
            return mat3.fromMat4(out, mat4.fromRotation(mat4.create(), radians, axis));
        }
        return fromRotationZ(out, radians);
    };
};

// ---------------
// Texture helpers
// ---------------

Filament._createTextureFromKtx1 = function(ktxdata, engine, options) {
    options = options || {};
    const ktx = options['ktx'] || new Filament.Ktx1Bundle(ktxdata);
    const srgb = !!options['srgb'];
    return Filament.ktx1reader$createTexture(engine, ktx, srgb);
};

Filament._createIblFromKtx1 = function(ktxdata, engine, options) {
    options = options || {};
    const iblktx = options['ktx'] = new Filament.Ktx1Bundle(ktxdata);

    const format = iblktx.info().glInternalFormat;
    //if (format != this.ctx.R11F_G11F_B10F && format != this.ctx.RGB16F && format != this.ctx.RGB32F) {
    if (format != 35898 && format != 33327 && format != 34837) {
        console.warn('IBL texture format is 0x' + format.toString(16) +
            ' which is not an expected floating-point format. Please use cmgen to generate IBL.');
    }

    const ibltex = Filament._createTextureFromKtx1(ktxdata, engine, options);
    const shstring = iblktx.getMetadata("sh");
    const ibl = Filament.IndirectLight.Builder()
        .reflections(ibltex)
        .build(engine);
    ibl.shfloats = shstring.split(/\s/, 9 * 3).map(parseFloat);
    return ibl;
};

Filament._createTextureFromImageFile = function(fileContents, engine, options) {
    const Sampler = Filament.Texture$Sampler;
    const TextureFormat = Filament.Texture$InternalFormat;
    const PixelDataFormat = Filament.PixelDataFormat;

    options = options || {};
    const srgb = !!options['srgb'];
    const noalpha = !!options['noalpha'];
    const nomips = !!options['nomips'];

    const decodedImage = Filament.decodeImage(fileContents, noalpha ? 3 : 4);

    let texformat, pbformat, pbtype;
    if (noalpha) {
        texformat = srgb ? TextureFormat.SRGB8 : TextureFormat.RGB8;
        pbformat = PixelDataFormat.RGB;
        pbtype = Filament.PixelDataType.UBYTE;
    } else {
        texformat = srgb ? TextureFormat.SRGB8_A8 : TextureFormat.RGBA8;
        pbformat = PixelDataFormat.RGBA;
        pbtype = Filament.PixelDataType.UBYTE;
    }

    const tex = Filament.Texture.Builder()
        .width(decodedImage.width)
        .height(decodedImage.height)
        .levels(nomips ? 1 : 0xff)
        .sampler(Sampler.SAMPLER_2D)
        .format(texformat)
        .build(engine);

    const pixelbuffer = Filament.PixelBuffer(decodedImage.data.getBytes(), pbformat, pbtype);
    tex.setImage(engine, 0, pixelbuffer);
    if (!nomips) {
        tex.generateMipmaps(engine);
    }
    return tex;
};

/// getSupportedFormats ::function:: Queries WebGL to check which compressed formats are supported.
/// ::retval:: object with boolean values and the following keys: s3tc, astc, etc
Filament.getSupportedFormats = function() {
    if (Filament.supportedFormats) {
        return Filament.supportedFormats;
    }
    const options = { majorVersion: 2, minorVersion: 0 };
    let ctx = document.createElement('canvas').getContext('webgl2', options);
    const result = {
        s3tc: false,
        s3tc_srgb: false,
        astc: false,
        etc: false,
    }
    let exts = ctx.getSupportedExtensions(), nexts = exts.length, i;
    for (i = 0; i < nexts; i++) {
        let ext = exts[i];
        if (ext == "WEBGL_compressed_texture_s3tc") {
            result.s3tc = true;
        } else if (ext == "WEBGL_compressed_texture_s3tc_srgb") {
            result.s3tc_srgb = true;
        } else if (ext == "WEBGL_compressed_texture_astc") {
            result.astc = true;
        } else if (ext == "WEBGL_compressed_texture_etc") {
            result.etc = true;
        }
    }
    return Filament.supportedFormats = result;
}

/// getSupportedFormatSuffix ::function:: Generate a file suffix according to the texture format.
/// Consumes a string describing desired formats and produces a file suffix depending on
/// which (if any) of the formats are actually supported by the WebGL implementation. This is
/// useful for compressed textures. For example, some platforms accept ETC and others accept S3TC.
/// desiredFormats ::argument:: space-delimited string of desired formats
/// ::retval:: empty string if there is no intersection of supported and desired formats.
Filament.getSupportedFormatSuffix = function(desiredFormats) {
    desiredFormats = desiredFormats.split(' ');
    let exts = Filament.getSupportedFormats();
    for (let key in exts) {
        if (exts[key] && desiredFormats.includes(key)) {
            return '_' + key;
        }
    }
    return '';
}
