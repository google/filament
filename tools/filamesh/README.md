# Filamesh

`filamesh` converts any mesh file supported by `assimp` (as configured in this source tree) into a
custom binary file format. The goal of this binary file format is to allow test applications to
easily and quickly load meshes.

The source mesh must have at least one set of UV coordinates.

The destination mesh will contain vertex positions, one set of UV coordinates and per-vertex
tangents, bitangents and normals.

The destination mesh is made of a single vertex buffer and a single index buffer. Mesh parts are
identified by an offset and count in the index buffer. Each part can have its own material.

## Usage

```
$ filamesh source_mesh destination_mesh
```

## Format

Note: the UV1 attribute cannot be used in interleaved mode

### Header

    char[8] : magic identifier "FILAMESH"
    uint32  : version number
    uint32  : number of parts (sub-meshes or draw calls)
    float3  : center of the total bounding box (AABB)
    float3  : half extent of the total bounding box (AABB)
    uint32  : flags (see below)
    uint32  : offset of the position attribute
    uint32  : stride of the position attribute
    uint32  : offset of the tangents attribute
    uint32  : stride of the tangents attribute
    uint32  : offset of the color attribute
    uint32  : stride of the color attribute
    uint32  : offset of the UV0 attribute
    uint32  : stride of the UV0 attribute
    uint32  : offset of the UV1 attribute (0xffffffff if UV1 is not present)
    uint32  : stride of the UV1 attribute (0xffffffff if UV1 is not present)
    uint32  : total number of vertices
    uint32  : size in bytes occupied by the (compressed) vertices
    uint32  : 0 if indices are stored as uint32, 1 if stored as uint16
    uint32  : total number of indices
    uint32  : size in bytes occupied by the (compressed) indices

The `flags` field contains the following bits:

- Bit 0: Specifies that vertex attributes are interleaved.
- Bit 1: UV's are 16-bit integers normalized into [-1, +1] rather than half-floats.
- Bit 2: Vertex and index data are compressed using zeux/meshoptimizer.

### Vertex data

    char*   : non-interleaved:
                  with n = number of vertices
                  n * half4:  XYZ positions, W set to 1.0
                  n * short4: tangent, bitangent and normal as a quaternion (snorm unsigned short)
                  n * ubyte4: color
                  n * half2:  UV texture coordinates
                  n * half2:  UV texture coordinates (if UV1 offset and stride != 0xffffffff)
              interleaved:
                  for each vertex:
                       half4:  XYZ position, W set to 1.0
                       short4: tangent, bitangent and normal as a quaternion (snorm unsigned short)
                       ubyte4: color
                       half2:  UV texture coordinates

### Index data

    char*   : each index is a uint32 or uint16 (see header)

### Parts

    for each part:
        uint32: offset of the first index in the index buffer
        uint32: number of indices that compose this part
        uint32: min index referenced by this part (glDrawRangeElements)
        uint32: max index referenced by this part (glDrawRangeElements)
        uint32: material ID (index in list of materials)
        float3: center of the part's bounding box (AABB)
        float3: half extent of the part's bounding box (AABB)

### Materials

    uint32  : number of materials
    for each material:
        uint32: length in bytes of the material name's string (not counting terminating \0)
        char* : name of the material (null terminated)

## Example

```c++
struct Mesh {
    utils::Entity renderable;
    VertexBuffer* vertexBuffer = nullptr;
    IndexBuffer* indexBuffer = nullptr;
};

struct Header {
    uint32_t version;
    uint32_t parts;
    Box      aabb;
    uint32_t flags;
    uint32_t offsetPosition;
    uint32_t stridePosition;
    uint32_t offsetTangents;
    uint32_t strideTangents;
    uint32_t offsetColor;
    uint32_t strideColor;
    uint32_t offsetUV0;
    uint32_t strideUV0;
    uint32_t offsetUV1;
    uint32_t strideUV1;
    uint32_t vertexCount;
    uint32_t vertexSize;
    uint32_t indexType;
    uint32_t indexCount;
    uint32_t indexSize;
};

struct Vertex {
    half4  position;
    short4 tangents;
    ubyte4 color;
    short2 uv0; // either half-float or snorm int16
};

struct Part {
    uint32_t offset;
    uint32_t indexCount;
    uint32_t minIndex;
    uint32_t maxIndex;
    uint32_t materialID;
    Box      aabb;
};

static size_t fileSize(int fd) {
    size_t filesize;
    filesize = (size_t) lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return filesize;
}

Mesh loadMeshFromFile(filament::Engine* engine, const utils::Path& path,
        const std::map<std::string, filament::MaterialInstance*>& materials) {

    Mesh mesh;

    int fd = open(path.c_str(), O_RDONLY);

    size_t size = fileSize(fd);
    char* data = (char*) mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (data) {
        char *p = data;

        char magic[9];
        memcpy(magic, (const char*) p, sizeof(char) * 8);
        magic[8] = '\0';
        p += sizeof(char) * 8;

        if (!strcmp("FILAMESH", magic)) {
            Header* header = (Header*) p;
            p += sizeof(Header);

            char* vertexData = p;
            p += header->vertexSize;

            char* indices = p;
            p += header->indexSize;

            Part* parts = (Part*) p;
            p += header->parts * sizeof(Part);

            uint32_t materialCount = (uint32_t) *p;
            p += sizeof(uint32_t);

            std::vector<std::string> partsMaterial;
            partsMaterial.resize(materialCount);

            for (size_t i = 0; i < materialCount; i++) {
                uint32_t nameLength = (uint32_t) *p;
                p += sizeof(uint32_t);

                partsMaterial[i] = p;
                p += nameLength + 1; // null terminated
            }

            mesh.indexBuffer = IndexBuffer::Builder()
                    .indexCount(header->indexCount)
                    .bufferType(header->indexType ? IndexBuffer::IndexType::USHORT
                                                  : IndexBuffer::IndexType::UINT)
                    .build(*engine);

            mesh.indexBuffer->setBuffer(*engine,
                    IndexBuffer::BufferDescriptor(indices, header->indexSize));

            const uint32_t FLAG_SNORM16_UV = 0x2;

            VertexBuffer::AttributeType::HALF2 uvType = VertexBuffer::AttributeType::HALF2;
            if (header->flags & FLAG_SNORM16_UV) {
                uvType = VertexBuffer::AttributeType::SHORT2;
            }
            bool uvNormalized = header->flags & FLAG_SNORM16_UV;

            VertexBuffer::Builder vbb;
            vbb.vertexCount(header->vertexCount)
                .bufferCount(1)
                .normalized(VertexAttribute::TANGENTS)
                .normalized(VertexAttribute::COLOR)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::HALF4,
                        header->offsetPosition, uint8_t(header->stridePosition))
                .attribute(VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::SHORT4,
                        header->offsetTangents, uint8_t(header->strideTangents))
                .attribute(VertexAttribute::COLOR,    0, VertexBuffer::AttributeType::UBYTE4,
                        header->offsetColor, uint8_t(header->strideColor))
                .attribute(VertexAttribute::UV0,      0, uvType,
                        header->offsetUV0, uint8_t(header->strideUV0))
                .normalized(VertexAttribute::UV0, uvNormalized);
            }

            if (header->offsetUV1 != std::numeric_limits<uint32_t>::max() &&
                    header->strideUV1 != std::numeric_limits<uint32_t>::max()) {
                vbb
                    .attribute(VertexAttribute::UV1, 0, uvType,
                            header->offsetUV1, uint8_t(header->strideUV1))
                   .normalized(VertexAttribute::UV1, uvNormalized);
            }

            mesh.vertexBuffer = vbb.build(*engine);

            VertexBuffer::BufferDescriptor buffer(vertexData, header->vertexSize);
            mesh.vertexBuffer->setBufferAt(*engine, 0, std::move(buffer));

            RenderableManager::Builder builder(header->parts);
            builder.boundingBox(header->aabb);

            for (size_t i = 0; i < header->parts; i++) {
                builder.geometry(i, RenderableManager::PrimitiveType::TRIANGLES,
                        mesh.vertexBuffer, mesh.indexBuffer, parts[i].offset,
                        parts[i].minIndex, parts[i].maxIndex, parts[i].indexCount);
                auto m = materials.find(partsMaterial[i]);
                if (m != materials.end()) {
                    builder.material(i, m->second);
                } else {
                    builder.material(i, materials.at("DefaultMaterial"));
                }
            }

            mesh.renderable = utils::EntityManager::get().create();
            builder.build(*engine, mesh.renderable);
        }

        Fence::waitAndDestroy(engine->createFence());
        munmap(data, size);
    }
    close(fd);

    return mesh;
}
```
