void main() {
#if defined(VERTEX_DOMAIN_DEVICE)
    gl_Position = getSkinnedPosition();
#else
    MaterialVertexInputs material;
    initMaterialVertex(material);
    gl_Position = getClipFromWorldMatrix() * material.worldPosition;
#endif
}
