/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <array>
#include <unordered_map>
#include <vector>
#include <fstream>

#include <filamat/MaterialBuilder.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <utils/EntityManager.h>
#include <backend/PixelBufferDescriptor.h>


namespace sample {

/**
 * @brief Type to represent a triangle mesh.
 */
struct TriangleMesh {
    std::vector<filament::math::float3> vertices; ///< 3D vertex positions.
    std::vector<filament::math::float3>
            colors; ///< Per-vertex colors, expected to be in RGB order and [0, 1].
    std::vector<filament::math::float3> vertex_normals; ///< The vertex normals of the mesh.
    std::vector<std::array<int, 3>> tvi;                ///< The triangle vertex indices.
};

} // namespace sample

namespace sample {

class Renderer {
private:
    filament::Engine* engine;

    filament::SwapChain* swap_chain;

    filament::Renderer* filament_renderer;
    filament::Scene* scene;
    filament::View* view;

    filament::IndexBuffer* index_buffer;
    filament::VertexBuffer* vertex_buffer;

    filament::Camera* cam;

    filament::Material* material;
    filament::MaterialInstance* material_instance;

    utils::Entity mesh_entity;
    utils::Entity camera_entity;

public:
    Renderer(int width, int height);

    Renderer(const Renderer& other) = delete;

    Renderer& operator=(const Renderer& other) = delete;

    void add_mesh(std::filesystem::path path_material, int mesh_grid_size_cols, int mesh_grid_size_rows);

    void render(int width, int height);

    ~Renderer();
};

} // namespace sample


namespace sample {

std::vector<std::uint8_t> read_filamat(const std::filesystem::path& filepath) {
    std::ifstream file(filepath.string(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filepath.string());
    }
    return buffer;
}

filament::math::float4 calculateTBN(const filament::math::float3& input_normal) {

    const filament::math::float3 normal = normalize(input_normal);

    filament::math::float3 picked_random_vector(1.0f, 0.0f, 0.0f);

    if (std::abs(dot(picked_random_vector, normal)) > 0.99f) {
        picked_random_vector = filament::math::float3(0.0f, 0.0f, 1.0f);
    }
    const filament::math::float3 tang = normalize(cross(picked_random_vector, normal));

    const filament::math::float3 bitang = normalize(cross(normal, tang));

    return filament::math::mat3f::packTangentFrame(filament::math::mat3f{ tang, bitang, normal })
            .xyzw;
}

Renderer::Renderer(const int width, const int height) {
    this->engine = filament::Engine::create(filament::Engine::Backend::VULKAN);

    this->filament_renderer = this->engine->createRenderer();
    this->scene = this->engine->createScene();
    this->view = this->engine->createView();
    this->swap_chain = this->engine->createSwapChain(width, height);
    this->view->setViewport(filament::Viewport(0, 0, width, height));
    this->view->setScene(this->scene);

    auto& em = utils::EntityManager::get();
    this->camera_entity = em.create();
    this->mesh_entity = em.create();
    this->cam = this->engine->createCamera(this->camera_entity);
}

Renderer::~Renderer() {
    this->scene->remove(this->mesh_entity);
    this->engine->destroy(this->mesh_entity);
    this->scene->forEach([](utils::Entity e) { std::cout << e.getId(); });
    this->engine->destroy(this->material_instance);
    this->engine->destroy(this->material);

    auto& em = utils::EntityManager::get();
    em.destroy(this->camera_entity);
    em.destroy(this->mesh_entity);
    this->mesh_entity.clear();

    this->engine->destroyCameraComponent(this->camera_entity);
    this->engine->destroy(this->camera_entity);
    this->engine->destroy(this->vertex_buffer);
    this->engine->destroy(this->index_buffer);

    this->engine->destroy(this->scene);
    this->engine->destroy(this->view);
    this->engine->destroy(this->swap_chain);
    this->engine->destroy(this->filament_renderer);
    this->engine->destroy(&this->engine);
    std::cout << "Destructed everything successfully." << std::endl;
}

void Renderer::add_mesh(const std::filesystem::path path_material, int mesh_grid_size_cols, int mesh_grid_size_rows) {
    // Create a triangle mesh with a few thousand vertices in the XY plane, CCW order:
    sample::TriangleMesh triangle_mesh;
    const int cols = mesh_grid_size_cols;
    const int rows = mesh_grid_size_rows;
    triangle_mesh.vertices.reserve(cols * rows);
    triangle_mesh.vertex_normals.reserve(cols * rows);
    triangle_mesh.colors.reserve(cols * rows);
    triangle_mesh.tvi.reserve((rows - 1) * (cols - 1) * 2);
    const float minX = -1.0f, maxX = 1.0f;
    const float minY = -1.0f, maxY = 1.0f;
    const float dx = (maxX - minX) / static_cast<float>(cols - 1);
    const float dy = (maxY - minY) / static_cast<float>(rows - 1);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float x = minX + c * dx;
            const float y = minY + r * dy;
            triangle_mesh.vertices.emplace_back(x, y, 0.0f);
            triangle_mesh.vertex_normals.emplace_back(0.0f, 0.0f, 1.0f);

            const float u = static_cast<float>(c) / static_cast<float>(cols - 1);
            const float v = static_cast<float>(r) / static_cast<float>(rows - 1);
            triangle_mesh.colors.emplace_back(u, v, 1.0f - u);
        }
    }
    for (int r = 0; r < rows - 1; ++r) {
        for (int c = 0; c < cols - 1; ++c) {
            // two CCW triangles per quad cell:
            const int i0 = r * cols + c;
            const int i1 = r * cols + (c + 1);
            const int i2 = (r + 1) * cols + c;
            const int i3 = (r + 1) * cols + (c + 1);
            // CCW for +Z
            triangle_mesh.tvi.push_back({ i0, i1, i2 });
            triangle_mesh.tvi.push_back({ i1, i3, i2 });
        }
    }

    const int count_vertices = triangle_mesh.vertices.size();
    const int count_indices = triangle_mesh.tvi.size() * 3;

    uint32_t* indices_arr = new uint32_t[triangle_mesh.tvi.size() * 3];
    for (int i = 0; i < triangle_mesh.tvi.size(); i++) {
        indices_arr[i * 3] = triangle_mesh.tvi[i][0];
        indices_arr[i * 3 + 1] = triangle_mesh.tvi[i][1];
        indices_arr[i * 3 + 2] = triangle_mesh.tvi[i][2];
    }

    filament::math::float3* vertices_f = new filament::math::float3[count_vertices];
    for (int i = 0; i < count_vertices; i++) {
        vertices_f[i] = triangle_mesh.vertices[i];
    }

    std::vector<float> colours_as_vector;
    for (int i = 0; i < triangle_mesh.colors.size(); i++) {
        colours_as_vector.push_back(triangle_mesh.colors[i][0]);
        colours_as_vector.push_back(triangle_mesh.colors[i][1]);
        colours_as_vector.push_back(triangle_mesh.colors[i][2]);
        colours_as_vector.push_back(1.f);
    }

    filament::math::float4* tbns = new filament::math::float4[count_vertices];
    for (size_t i = 0; i < count_vertices; i++) {
        tbns[i] = calculateTBN(triangle_mesh.vertex_normals[i]);
    }

    this->vertex_buffer = filament::VertexBuffer::Builder()
                                  .vertexCount(count_vertices)
                                  .bufferCount(3)
                                  .attribute(filament::VertexAttribute::POSITION, 0,
                                          filament::VertexBuffer::AttributeType::FLOAT3)
                                  .attribute(filament::VertexAttribute::TANGENTS, 1,
                                          filament::VertexBuffer::AttributeType::FLOAT4)
                                  .attribute(filament::VertexAttribute::COLOR, 2,
                                          filament::VertexBuffer::AttributeType::FLOAT4)
                                  .build(*this->engine);

    this->vertex_buffer->setBufferAt(*this->engine, 0,
            filament::VertexBuffer::BufferDescriptor(vertices_f,
                    this->vertex_buffer->getVertexCount() * sizeof(vertices_f[0])));
    this->vertex_buffer->setBufferAt(*this->engine, 1,
            filament::VertexBuffer::BufferDescriptor(tbns,
                    this->vertex_buffer->getVertexCount() * sizeof(tbns[0])));
    this->vertex_buffer->setBufferAt(*this->engine, 2,
            filament::VertexBuffer::BufferDescriptor(colours_as_vector.data(),
                    colours_as_vector.size() * sizeof(colours_as_vector[0])));

    this->index_buffer =
            filament::IndexBuffer::Builder().indexCount(count_indices).build(*this->engine);

    this->index_buffer->setBuffer(*this->engine,
            filament::IndexBuffer::BufferDescriptor(indices_arr,
                    this->index_buffer->getIndexCount() * sizeof(uint32_t)));

    const std::vector<uint8_t> materialData = read_filamat(path_material);
    this->material = filament::Material::Builder()
                             .package(materialData.data(), materialData.size())
                             .build(*this->engine);
    this->material_instance = this->material->createInstance();
    this->material_instance->setParameter("baseColor", filament::RgbType::sRGB,
            filament::math::float3{ 75.0, 75.0, 75.0 });

    filament::RenderableManager::Builder(1)
            .boundingBox({ { -1.5, -1.5, -1.5 }, { 1.5, 1.5, 1.5 } })
            .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, this->vertex_buffer,
                    this->index_buffer, 0, count_indices)
            .material(0, this->material_instance)
            .culling(false)
            .receiveShadows(true)
            .castShadows(true)
            .build(*this->engine, this->mesh_entity);
    this->scene->addEntity(this->mesh_entity);
}

struct DataMessage {
    std::string name;
    int width;
    int height;
};

void Renderer::render(const int width, const int height) {
    const float aspect = (float) width / height;
    const double nr = 0.1f, fr = 200.0f;
    cam->setProjection(45., aspect, nr, fr);

    cam->lookAt({ 0., 0., 0. }, { 0., 0., -1. }, { 0., 1., 0. });

    this->view->setCamera(cam);

    // Rendering:
    size_t size = width * height * 4;
    uint8_t* buffer = new uint8_t[size];

    filament::backend::PixelBufferDescriptor pd(
            buffer, size, filament::backend::PixelBufferDescriptor::PixelDataFormat::RGBA,
            filament::backend::PixelBufferDescriptor::PixelDataType::UBYTE,
            [](void* buffer, size_t size, void* user) {
                {
                    // std::cout << "Here" << std::endl;
                    // auto* message = static_cast<DataMessage*>(user);
                    // cv::Mat image = cv::Mat(message->height, message->width, CV_8UC4,
                    // static_cast<uint8_t*>(buffer)).clone(); image.convertTo(image, CV_8U);
                    // cv::cvtColor(image, image, cv::COLOR_RGBA2RGB);
                    // cv::imwrite(message->name, image);

                    // delete[] static_cast<uint8_t*>(buffer);
                    // delete message;
                }
            },
            new DataMessage{ "aaa.png", width, height });

    const bool should_draw_frame = this->filament_renderer->beginFrame(this->swap_chain);
    if (should_draw_frame) {
        this->filament_renderer->render(this->view);
        // this->filament_renderer->readPixels(0, 0, width, height, std::move(pd));
        this->filament_renderer->endFrame();
        // this->engine->flushAndWait();
    }
}
} // namespace sample


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: issue_9062_repro <path-to-material.filamat>\n";
        return EXIT_FAILURE;
    }
    const std::filesystem::path material_path = argv[1];
    if (!std::filesystem::exists(material_path) || !std::filesystem::is_regular_file(material_path)) {
        std::cerr << "Error: material file not found: " << material_path << "\n";
        return EXIT_FAILURE;
    }

    const int width = 1000;
    const int height = 1000;

    auto renderer = std::make_unique<sample::Renderer>(width, height);
    renderer->add_mesh(material_path, 1000, 800);

    int num_frames = 1;
    for (std::size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
        std::cout << "Rendering frame " << frame_idx + 1 << " of " << num_frames << "\n";
        renderer->render(width, height);
    }

    return EXIT_SUCCESS;
}
