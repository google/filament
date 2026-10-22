// This file is part of meshoptimizer library and is distributed under the terms of MIT License.
// Copyright (C) 2016-2022, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
export const MeshoptEncoder: {
    supported: boolean;
    ready: Promise<void>;

    reorderMesh: (indices: Uint32Array, triangles: boolean, optsize: boolean) => [Uint32Array, number];
    
    encodeVertexBuffer: (source: Uint8Array, count: number, size: number) => Uint8Array;
    encodeIndexBuffer: (source: Uint8Array, count: number, size: number) => Uint8Array;
    encodeIndexSequence: (source: Uint8Array, count: number, size: number) => Uint8Array;

    encodeGltfBuffer: (source: Uint8Array, count: number, size: number, mode: string) => Uint8Array;

    encodeFilterOct: (source: Float32Array, count: number, stride: number, bits: number) => Uint8Array;
    encodeFilterQuat: (source: Float32Array, count: number, stride: number, bits: number) => Uint8Array;
    encodeFilterExp: (source: Float32Array, count: number, stride: number, bits: number) => Uint8Array;
};
