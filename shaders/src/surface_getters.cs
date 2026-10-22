//------------------------------------------------------------------------------
// Compute builtin access
//------------------------------------------------------------------------------

/**
 * @public-api
 * Work group the thread belongs to.
 */
uvec3 getWorkGroupID() {
    return gl_WorkGroupID;
}

/**
 * @public-api
 * Number of work groups in each dimensions
 */
uvec3 getWorkGroupCount() {
    return gl_NumWorkGroups;
}

/**
 * @public-api
 * Within the work group, get a unique identifier for the thread.
 */
uvec3 getLocalInvocationID() {
    return gl_LocalInvocationID;
}

/**
 * @public-api
 * Globally unique value across the entire compute dispatch.
 * 1D version of getLocalInvocationID(). Provided for convenience.
 */
uint getLocalInvocationIndex() {
    return gl_LocalInvocationIndex;
}

/**
 * @public-api
 * Globally unique value across the entire compute dispatch.
 * Short-hand for getWorkGroupID() * getWorkGroupCount() + getLocalInvocationID()
 */
uvec3 getGlobalInvocationID() {
    return gl_GlobalInvocationID;
}
