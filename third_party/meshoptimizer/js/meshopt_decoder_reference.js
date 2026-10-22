// This file is part of meshoptimizer library and is distributed under the terms of MIT License.
// Copyright (C) 2016-2022, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)

// This is the reference decoder implementation by Jasper St. Pierre.
// It follows the decoder interface and should be a drop-in replacement for the actual decoder from meshopt_decoder.js
// It is provided for educational value and is not recommended for use in production because it's not performance-optimized.

const MeshoptDecoder = {};
MeshoptDecoder.supported = true;
MeshoptDecoder.ready = Promise.resolve();

function assert(cond) {
    if (!cond) {
        throw new Error("Assertion failed");
    }
}

function dezig(v) {
    return ((v & 1) !== 0) ? ~(v >>> 1) : v >>> 1;
}

MeshoptDecoder.decodeVertexBuffer = (target, elementCount, byteStride, source, filter) => {
    assert(source[0] === 0xA0);

    const maxBlockElements = Math.min((0x2000 / byteStride) & ~0x000F, 0x100);

    const deltas = new Uint8Array(0x10);

    const tailDataOffs = source.length - byteStride;

    // What deltas are stored relative to
    const tempData = source.slice(tailDataOffs, tailDataOffs + byteStride);

    let srcOffs = 0x01;

    // Attribute Blocks
    for (let dstElemBase = 0; dstElemBase < elementCount; dstElemBase += maxBlockElements) {
        const attrBlockElementCount = Math.min(elementCount - dstElemBase, maxBlockElements);
        const groupCount = ((attrBlockElementCount + 0x0F) & ~0x0F) >>> 4;
        const headerByteCount = ((groupCount + 0x03) & ~0x03) >>> 2;

        // Data blocks
        for (let byte = 0; byte < byteStride; byte++) {
            let headerBitsOffs = srcOffs;

            srcOffs += headerByteCount;
            for (let group = 0; group < groupCount; group++) {
                const mode = (source[headerBitsOffs] >>> ((group & 0x03) << 1)) & 0x03;
                // If this is the last group, move to the next byte of header bits.
                if ((group & 0x03) === 0x03)
                    headerBitsOffs++;

                const dstElemGroup = dstElemBase + (group << 4);

                if (mode === 0) {
                    // bits 0: All 16 byte deltas are 0; the size of the encoded block is 0 bytes
                    deltas.fill(0x00);
                } else if (mode === 1) {
                    // bits 1: Deltas are using 2-bit sentinel encoding; the size of the encoded block is [4..20] bytes
                    const srcBase = srcOffs;
                    srcOffs += 0x04;
                    for (let m = 0; m < 0x10; m++) {
                        // 0 = >>> 6, 1 = >>> 4, 2 = >>> 2, 3 = >>> 0
                        const shift = (6 - ((m & 0x03) << 1));
                        let delta = (source[srcBase + (m >>> 2)] >>> shift) & 0x03;
                        if (delta === 3)
                            delta = source[srcOffs++];
                        deltas[m] = delta;
                    }
                } else if (mode === 2) {
                    // bits 2: Deltas are using 4-bit sentinel encoding; the size of the encoded block is [8..24] bytes
                    const srcBase = srcOffs;
                    srcOffs += 0x08;
                    for (let m = 0; m < 0x10; m++) {
                        // 0 = >>> 6, 1 = >>> 4, 2 = >>> 2, 3 = >>> 0
                        const shift = (m & 0x01) ? 0 : 4;
                        let delta = (source[srcBase + (m >>> 1)] >>> shift) & 0x0f;
                        if (delta === 0xf)
                            delta = source[srcOffs++];
                        deltas[m] = delta;
                    }
                } else {
                    // bits 3: All 16 byte deltas are stored verbatim; the size of the encoded block is 16 bytes
                    deltas.set(source.subarray(srcOffs, srcOffs + 0x10));
                    srcOffs += 0x10;
                }

                // Go through and apply deltas to data
                for (let m = 0; m < 0x10; m++) {
                    const dstElem = dstElemGroup + m;
                    if (dstElem >= elementCount)
                        break;

                    const delta = dezig(deltas[m]);
                    const dstOffs = dstElem * byteStride + byte;
                    target[dstOffs] = (tempData[byte] += delta);
                }
            }
        }
    }

    // Filters - only applied if filter isn't undefined or NONE
    if (filter === 'OCTAHEDRAL') {
        assert(byteStride === 4 || byteStride === 8);

        let dst, maxInt;
        if (byteStride === 4) {
            dst = new Int8Array(target.buffer);
            maxInt = 127;
        } else {
            dst = new Int16Array(target.buffer);
            maxInt = 32767;
        }

        for (let i = 0; i < 4 * elementCount; i += 4) {
            let x = dst[i + 0], y = dst[i + 1], one = dst[i + 2];
            x /= one;
            y /= one;
            const z = 1.0 - Math.abs(x) - Math.abs(y);
            const t = Math.max(-z, 0.0);
            x -= (x >= 0) ? t : -t;
            y -= (y >= 0) ? t : -t;
            const h = maxInt / Math.hypot(x, y, z);
            dst[i + 0] = Math.round(x * h);
            dst[i + 1] = Math.round(y * h);
            dst[i + 2] = Math.round(z * h);
            // keep dst[i + 3] as is
        }
    } else if (filter === 'QUATERNION') {
        assert(byteStride === 8);

        const dst = new Int16Array(target.buffer);

        for (let i = 0; i < 4 * elementCount; i += 4) {
            const inputW = dst[i + 3];
            const maxComponent = inputW & 0x03;
            const s = Math.SQRT1_2 / (inputW | 0x03);
            let x = dst[i + 0] * s;
            let y = dst[i + 1] * s;
            let z = dst[i + 2] * s;
            let w = Math.sqrt(Math.max(0.0, 1.0 - x**2 - y**2 - z**2));
            dst[i + (maxComponent + 1) % 4] = Math.round(x * 32767);
            dst[i + (maxComponent + 2) % 4] = Math.round(y * 32767);
            dst[i + (maxComponent + 3) % 4] = Math.round(z * 32767);
            dst[i + (maxComponent + 0) % 4] = Math.round(w * 32767);
        }
    } else if (filter === 'EXPONENTIAL') {
        assert((byteStride & 0x03) === 0x00);

        const src = new Int32Array(target.buffer);
        const dst = new Float32Array(target.buffer);
        for (let i = 0; i < (byteStride * elementCount) / 4; i++) {
            const v = src[i], exp = v >> 24, mantissa = (v << 8) >> 8;
            dst[i] = 2.0**exp * mantissa;
        }
    }
};

function pushfifo(fifo, n) {
    for (let i = fifo.length - 1; i > 0; i--)
        fifo[i] = fifo[i - 1];
    fifo[0] = n;
}

MeshoptDecoder.decodeIndexBuffer = (target, count, byteStride, source) => {
    assert(source[0] === 0xE1);
    assert(count % 3 === 0);
    assert(byteStride === 2 || byteStride === 4);

    let dst;
    if (byteStride === 2)
        dst = new Uint16Array(target.buffer);
    else
        dst = new Uint32Array(target.buffer);

    const triCount = count / 3;

    let codeOffs = 0x01;
    let dataOffs = codeOffs + triCount;
    let codeauxOffs = source.length - 0x10;

    function readLEB128() {
        let n = 0;
        for (let i = 0; ; i += 7) {
            const b = source[dataOffs++];
            n |= (b & 0x7F) << i;

            if (b < 0x80)
                return n;
        }
    }

    let next = 0, last = 0;
    const edgefifo = new Uint32Array(32);
    const vertexfifo = new Uint32Array(16);

    function decodeIndex(v) {
        return (last += dezig(v));
    }

    let dstOffs = 0;
    for (let i = 0; i < triCount; i++) {
        const code = source[codeOffs++];
        const b0 = code >>> 4, b1 = code & 0x0F;

        if (b0 < 0x0F) {
            const a = edgefifo[(b0 << 1) + 0], b = edgefifo[(b0 << 1) + 1];
            let c = -1;

            if (b1 === 0x00) {
                c = next++;
                pushfifo(vertexfifo, c);
            } else if (b1 < 0x0D) {
                c = vertexfifo[b1];
            } else if (b1 === 0x0D) {
                c = --last;
                pushfifo(vertexfifo, c);
            } else if (b1 === 0x0E) {
                c = ++last;
                pushfifo(vertexfifo, c);
            } else if (b1 === 0x0F) {
                const v = readLEB128();
                c = decodeIndex(v);
                pushfifo(vertexfifo, c);
            }

            // fifo pushes happen backwards
            pushfifo(edgefifo, b); pushfifo(edgefifo, c);
            pushfifo(edgefifo, c); pushfifo(edgefifo, a);

            dst[dstOffs++] = a;
            dst[dstOffs++] = b;
            dst[dstOffs++] = c;
        } else { // b0 === 0x0F
            let a = -1, b = -1, c = -1;

            if (b1 < 0x0E) {
                const e = source[codeauxOffs + b1];
                const z = e >>> 4, w = e & 0x0F;

                a = next++;

                if (z === 0x00)
                    b = next++;
                else
                    b = vertexfifo[z - 1];

                if (w === 0x00)
                    c = next++;
                else
                    c = vertexfifo[w - 1];

                pushfifo(vertexfifo, a);
                if (z === 0x00)
                    pushfifo(vertexfifo, b);
                if (w === 0x00)
                    pushfifo(vertexfifo, c);
            } else {
                const e = source[dataOffs++];
                if (e === 0x00)
                    next = 0;

                const z = e >>> 4, w = e & 0x0F;

                if (b1 === 0x0E)
                    a = next++;
                else
                    a = decodeIndex(readLEB128());

                if (z === 0x00)
                    b = next++;
                else if (z === 0x0F)
                    b = decodeIndex(readLEB128());
                else
                    b = vertexfifo[z - 1];

                if (w === 0x00)
                    c = next++;
                else if (w === 0x0F)
                    c = decodeIndex(readLEB128());
                else
                    c = vertexfifo[w - 1];

                pushfifo(vertexfifo, a);
                if (z === 0x00 || z === 0x0F)
                    pushfifo(vertexfifo, b);
                if (w === 0x00 || w === 0x0F)
                    pushfifo(vertexfifo, c);
            }

            pushfifo(edgefifo, a); pushfifo(edgefifo, b);
            pushfifo(edgefifo, b); pushfifo(edgefifo, c);
            pushfifo(edgefifo, c); pushfifo(edgefifo, a);

            dst[dstOffs++] = a;
            dst[dstOffs++] = b;
            dst[dstOffs++] = c;
        }
    }
};

MeshoptDecoder.decodeIndexSequence = (target, count, byteStride, source) => {
    assert(source[0] === 0xD1);
    assert(byteStride === 2 || byteStride === 4);

    let dst;
    if (byteStride === 2)
        dst = new Uint16Array(target.buffer);
    else
        dst = new Uint32Array(target.buffer);

    let dataOffs = 0x01;

    function readLEB128() {
        let n = 0;
        for (let i = 0; ; i += 7) {
            const b = source[dataOffs++];
            n |= (b & 0x7F) << i;

            if (b < 0x80)
                return n;
        }
    }

    const last = new Uint32Array(2);

    for (let i = 0; i < count; i++) {
        const v = readLEB128();
        const b = (v & 0x01);
        const delta = dezig(v >>> 1);
        dst[i] = (last[b] += delta);
    }
};

MeshoptDecoder.decodeGltfBuffer = (target, count, size, source, mode, filter) => {
    var table = {
        ATTRIBUTES: MeshoptDecoder.decodeVertexBuffer,
        TRIANGLES: MeshoptDecoder.decodeIndexBuffer,
        INDICES: MeshoptDecoder.decodeIndexSequence,
    };
    assert(table[mode] !== undefined);
    table[mode](target, count, size, source, filter);
};

// node.js interface:
// for (let k in MeshoptDecoder) { exports[k] = MeshoptDecoder[k]; }

export { MeshoptDecoder };
