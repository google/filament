// RUN: %dxc -T lib_6_x -Od %s | FileCheck %s

// lib_6_x allows phi on resource, targeting offline linking only.
// CHECK: phi %dx.types.Handle

ByteAddressBuffer firstBuffer, secondBuffer;
uint firstBufferSize;

uint load(uint offset)
{
    ByteAddressBuffer buffer = firstBuffer;
    if (offset > firstBufferSize) {
        // If we just do this assignment, we'll generate a select instead of a phi
        buffer = secondBuffer;
        offset -= firstBufferSize;
    }

    return buffer.Load(offset);
}