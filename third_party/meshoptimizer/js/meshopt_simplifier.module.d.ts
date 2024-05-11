// This file is part of meshoptimizer library and is distributed under the terms of MIT License.
// Copyright (C) 2016-2022, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
export type Flags = "LockBorder";

export const MeshoptSimplifier: {
    supported: boolean;
    ready: Promise<void>;
    
    compactMesh: (indices: Uint32Array) => [Uint32Array, number];
    
    simplify: (indices: Uint32Array, vertex_positions: Float32Array, vertex_positions_stride: number, target_index_count: number, target_error: number, flags?: Flags[]) => [Uint32Array, number];

    getScale: (vertex_positions: Float32Array, vertex_positions_stride: number) => number;
};
