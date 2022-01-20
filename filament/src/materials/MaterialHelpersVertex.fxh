//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions for Visualization material implementations
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Vertex processing functions
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessVertex(inout MaterialVertexInputs material) {
#if defined(SHAPR_OBJECT_SPACE_TRILINEAR)
    //
    // This is intentionally not compatible with skinning
    //
    material.objectSpacePosition = vec4(mesh_position.xyz, 1);

    // Attribute mesh_tangents is a vec4 input quaternion, encoding our tangent frame. As such, even though
    // the fixed parts of the Filament shader skeleton compute everything in world space, by applying the
    // world transforms, mesh_tangents is still the data from the vertex buffer
    highp vec3 normalOS; // object space normal
    toTangentFrame(mesh_tangents, normalOS);
    material.objectSpaceNormal = vec4(normalOS.xyz, 0);
#endif
}