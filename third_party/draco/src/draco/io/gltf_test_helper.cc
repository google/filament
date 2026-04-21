// Copyright 2022 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco/io/gltf_test_helper.h"

#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "draco/attributes/geometry_attribute.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/metadata/property_attribute.h"
#include "draco/metadata/property_table.h"
#include "draco/texture/texture_library.h"

namespace draco {

#ifdef DRACO_TRANSCODER_SUPPORTED

void GltfTestHelper::AddBoxMetaMeshFeatures(Scene *scene) {
  // Check the scene.
  ASSERT_NE(scene, nullptr);
  ASSERT_EQ(scene->NumMeshes(), 1);
  TextureLibrary &texture_library = scene->GetNonMaterialTextureLibrary();
  ASSERT_EQ(texture_library.NumTextures(), 0);

  // Check the mesh.
  Mesh &mesh = scene->GetMesh(MeshIndex(0));
  ASSERT_EQ(mesh.num_faces(), 12);
  ASSERT_EQ(mesh.num_attributes(), 2);
  ASSERT_EQ(mesh.num_points(), 24);

  // Get mesh element counts.
  const int num_faces = mesh.num_faces();
  const int num_corners = 3 * mesh.num_faces();
  const int num_vertices =
      mesh.GetNamedAttribute(GeometryAttribute::POSITION)->size();

  // Add feature ID set with per-face Uint8 attribute named _FEATURE_ID_0.
  {
    // Create feature ID attribute.
    constexpr DataType kType = DataType::DT_UINT8;
    std::unique_ptr<PointAttribute> pa(new PointAttribute());
    pa->Init(GeometryAttribute::GENERIC, 1, kType, false, mesh.num_faces());
    for (AttributeValueIndex avi(0); avi < num_faces; ++avi) {
      const int8_t val = avi.value();
      pa->SetAttributeValue(avi, &val);
    }
    const int att_id = mesh.AddPerFaceAttribute(std::move(pa));

    // Add feature ID set to the mesh.
    std::unique_ptr<MeshFeatures> features(new MeshFeatures());
    features->SetLabel("faces");
    features->SetFeatureCount(num_faces);
    features->SetNullFeatureId(100);
    features->SetPropertyTableIndex(0);
    features->SetAttributeIndex(att_id);
    mesh.AddMeshFeatures(std::move(features));
  }

  // Add feature ID set with per-vertex Uint16 attribute named _FEATURE_ID_1.
  {
    // Create feature ID attribute.
    constexpr DataType kType = DataType::DT_UINT16;
    std::unique_ptr<PointAttribute> pa(new PointAttribute());
    pa->Init(GeometryAttribute::GENERIC, 1, kType, false, num_vertices);
    for (AttributeValueIndex avi(0); avi < num_vertices; ++avi) {
      const uint16_t val = avi.value();
      pa->SetAttributeValue(avi, &val);
    }
    const int att_id = mesh.AddPerVertexAttribute(std::move(pa));

    // Add feature ID set to the mesh.
    std::unique_ptr<MeshFeatures> features(new MeshFeatures());
    features->SetLabel("vertices");
    features->SetFeatureCount(num_vertices);
    features->SetNullFeatureId(101);
    features->SetPropertyTableIndex(1);
    features->SetAttributeIndex(att_id);
    mesh.AddMeshFeatures(std::move(features));
  }

  // Add feature ID set with per-corner Float attribute named _FEATURE_ID_2.
  {
    // Create feature ID attribute.
    constexpr DataType kType = DataType::DT_FLOAT32;
    std::unique_ptr<PointAttribute> pa(new PointAttribute());
    pa->Init(GeometryAttribute::GENERIC, 1, kType, false, num_corners);
    IndexTypeVector<CornerIndex, AttributeValueIndex> corner_to_value(
        num_corners);
    for (AttributeValueIndex avi(0); avi < num_corners; ++avi) {
      const float val = avi.value();
      pa->SetAttributeValue(avi, &val);
      corner_to_value[CornerIndex(avi.value())] = avi;
    }
    const int att_id =
        mesh.AddAttributeWithConnectivity(std::move(pa), corner_to_value);

    // Add feature ID set to the mesh.
    std::unique_ptr<MeshFeatures> features(new MeshFeatures());
    features->SetFeatureCount(num_corners);
    features->SetAttributeIndex(att_id);
    mesh.AddMeshFeatures(std::move(features));
  }

  // Add feature ID set with the IDs stored in the R texture channel and
  // accessible via the first texture coordinate attribute.
  {
    // Add the first texture coordinate attribute.
    constexpr DataType kType = DataType::DT_FLOAT32;
    std::unique_ptr<PointAttribute> pa(new PointAttribute());
    pa->Init(GeometryAttribute::TEX_COORD, 2, kType, false, num_vertices);
    std::vector<std::array<float, 2>> uv = {
        {0.0000f, 0.0000f}, {0.0000f, 0.5000f}, {0.0000f, 1.0000f},
        {0.5000f, 0.0000f}, {0.5000f, 0.5000f}, {0.5000f, 1.0000f},
        {1.0000f, 0.0000f}, {1.0000f, 0.5000f}};
    for (AttributeValueIndex avi(0); avi < num_vertices; ++avi) {
      const int index = avi.value();
      pa->SetAttributeValue(avi, uv[index].data());
    }
    mesh.AddPerVertexAttribute(std::move(pa));
  }

  // Add feature ID set with the IDs stored in the GBA texture channels and
  // accessible via the second texture coordinate attribute.
  {
    // Add the second texture coordinate attribute.
    constexpr DataType kType = DataType::DT_FLOAT32;
    std::unique_ptr<PointAttribute> pa(new PointAttribute());
    pa->Init(GeometryAttribute::TEX_COORD, 2, kType, false, num_vertices);
    std::vector<std::array<float, 2>> uv = {
        {0.0000f, 0.0000f}, {0.0000f, 0.5000f}, {0.0000f, 1.0000f},
        {0.5000f, 0.0000f}, {0.5000f, 0.5000f}, {0.5000f, 1.0000f},
        {1.0000f, 0.0000f}, {1.0000f, 0.5000f}};
    for (AttributeValueIndex avi(0); avi < num_vertices; ++avi) {
      const int index = avi.value();
      pa->SetAttributeValue(avi, uv[index].data());
    }
    mesh.AddPerVertexAttribute(std::move(pa));
    ASSERT_EQ(mesh.NumNamedAttributes(GeometryAttribute::TEX_COORD), 2);
  }
}

void GltfTestHelper::AddBoxMetaStructuralMetadata(Scene *scene) {
  // Add structural metadata schema in the following JSON:
  // "schema": {
  //   "id": "galaxy",
  //   "classes": {
  //     "planet": {
  //       "properties": {
  //         "color": {
  //           "componentType": "UINT8",
  //           "description": "The RGB color.",
  //           "required": true,
  //           "type": "VEC3"
  //         },
  //         "name": {
  //           "description": "The name.",
  //           "required": true,
  //           "type": "STRING"
  //         }
  //         "sequence": {
  //           "componentType": "FLOAT32",
  //           "description": "The number sequence.",
  //           "required": false,
  //           "type": "SCALAR"
  //         }
  //       }
  //     },
  //     "movement": {
  //       "name": "The movement.",
  //       "description": "Vertex movement.",
  //       "properties": {
  //         "direction": {
  //           "description": "Movement direction.",
  //           "type": "VEC3",
  //           "componentType": "FLOAT32",
  //           "required": true
  //         },
  //         "magnitude": {
  //           "description": "Movement magnitude.",
  //           "type": "SCALAR",
  //           "componentType": "FLOAT32",
  //           "required": true
  //         }
  //       }
  //     }
  //   },
  //   "enums": {
  //     "classifications": {
  //       "description": "Classifications of planets.",
  //       "name": "classifications",
  //       "values": [
  //         { "name": "Unspecified", "value": 0 },
  //         { "name": "Gas Giant", "value": 1 },
  //         { "name": "Waterworld", "value": 2 },
  //         { "name": "Agriworld", "value": 3 },
  //         { "name": "Ordnance", "value": 4 }
  //       ]
  //     }
  //   }
  // },
  // "propertyAttributes": [{
  //   "name": "The movement.",
  //   "class": "movement",
  //   "properties": {
  //     "direction": {
  //       "attribute": "_DIRECTION",
  //     },
  //     "magnitude": {
  //       "attribute": "_MAGNITUDE",
  //     }
  //   }
  // }]
  typedef StructuralMetadataSchema::Object Object;
  StructuralMetadataSchema schema;
  Object &json = schema.json;
  json.SetObjects().emplace_back("id", "galaxy");
  json.SetObjects().emplace_back("classes");

  // Add class "planet" to schema.
  {
    json.SetObjects().back().SetObjects().emplace_back("planet");
    Object &planet = json.SetObjects().back().SetObjects().back();
    planet.SetObjects().emplace_back("properties");
    Object &properties = planet.SetObjects().back();

    properties.SetObjects().emplace_back("color");
    Object &color = properties.SetObjects().back();
    color.SetObjects().emplace_back("componentType", "UINT8");
    color.SetObjects().emplace_back("description", "The RGB color.");
    color.SetObjects().emplace_back("required", true);
    color.SetObjects().emplace_back("type", "VEC3");

    properties.SetObjects().emplace_back("name");
    Object &name = properties.SetObjects().back();
    name.SetObjects().emplace_back("description", "The name.");
    name.SetObjects().emplace_back("required", true);
    name.SetObjects().emplace_back("type", "STRING");

    properties.SetObjects().emplace_back("sequence");
    Object &sequence = properties.SetObjects().back();
    sequence.SetObjects().emplace_back("componentType", "FLOAT32");
    sequence.SetObjects().emplace_back("description", "The number sequence.");
    sequence.SetObjects().emplace_back("required", false);
    sequence.SetObjects().emplace_back("type", "SCALAR");
  }

  // Add class "movement" to schema.
  {
    json.SetObjects().back().SetObjects().emplace_back("movement");
    Object &movement = json.SetObjects().back().SetObjects().back();
    movement.SetObjects().emplace_back("name", "The movement.");
    movement.SetObjects().emplace_back("description", "Vertex movement.");
    movement.SetObjects().emplace_back("properties");
    Object &properties = movement.SetObjects().back();

    properties.SetObjects().emplace_back("direction");
    Object &direction = properties.SetObjects().back();
    direction.SetObjects().emplace_back("componentType", "FLOAT32");
    direction.SetObjects().emplace_back("description", "Movement direction.");
    direction.SetObjects().emplace_back("required", true);
    direction.SetObjects().emplace_back("type", "VEC3");

    properties.SetObjects().emplace_back("magnitude");
    Object &mag = properties.SetObjects().back();
    mag.SetObjects().emplace_back("componentType", "FLOAT32");
    mag.SetObjects().emplace_back("description", "Movement magnitude.");
    mag.SetObjects().emplace_back("required", true);
    mag.SetObjects().emplace_back("type", "SCALAR");
  }

  json.SetObjects().emplace_back("enums");
  json.SetObjects().back().SetObjects().emplace_back("classifications");
  Object &classifications = json.SetObjects().back().SetObjects().back();
  classifications.SetObjects().emplace_back("description",
                                            "Classifications of planets.");
  classifications.SetObjects().emplace_back("name", "classifications");
  classifications.SetObjects().emplace_back("values");
  Object &values = classifications.SetObjects().back();

  values.SetArray().emplace_back();
  values.SetArray().back().SetObjects().emplace_back("name", "Unspecified");
  values.SetArray().back().SetObjects().emplace_back("value", 0);

  values.SetArray().emplace_back();
  values.SetArray().back().SetObjects().emplace_back("name", "Gas Giant");
  values.SetArray().back().SetObjects().emplace_back("value", 1);

  values.SetArray().emplace_back();
  values.SetArray().back().SetObjects().emplace_back("name", "Waterworld");
  values.SetArray().back().SetObjects().emplace_back("value", 2);

  values.SetArray().emplace_back();
  values.SetArray().back().SetObjects().emplace_back("name", "Agriworld");
  values.SetArray().back().SetObjects().emplace_back("value", 3);

  values.SetArray().emplace_back();
  values.SetArray().back().SetObjects().emplace_back("name", "Ordnance");
  values.SetArray().back().SetObjects().emplace_back("value", 4);

  // Add structural metadata schema to the scene.
  scene->GetStructuralMetadata().SetSchema(schema);

  // Add structural metadata property table.
  std::unique_ptr<PropertyTable> table(new PropertyTable());
  table->SetName("Galaxy far far away.");
  table->SetClass("planet");
  table->SetCount(16);

  // Add property describing RGB color components of the planet class.
  {
    std::unique_ptr<PropertyTable::Property> property(
        new PropertyTable::Property());
    property->SetName("color");
    property->GetData().target = 34962;         // ARRAY_BUFFER.
    property->GetData().data = {94,  94,  194,  // Tatooine
                                94,  145, 161,  // Corusant
                                118, 171, 91,   // Naboo
                                103, 139, 178,  // Alderaan
                                83,  98,  154,  // Dagobah
                                91,  177, 175,  // Mandalore
                                190, 92,  108,  // Corellia
                                72,  69,  169,  // Kamino
                                154, 90,  101,  // Kashyyyk
                                174, 85,  175,  // Dantooine
                                184, 129, 96,   // Hoth
                                185, 91,  180,  // Mustafar
                                194, 150, 83,   // Bespin
                                204, 111, 134,  // Yavin
                                182, 90,  89,   // Geonosis
                                0,   0,   0};   // UNLABELED
    table->AddProperty(std::move(property));
  }

  // Add property that describes names of the planet class.
  {
    std::unique_ptr<PropertyTable::Property> property(
        new PropertyTable::Property());
    property->SetName("name");
    property->GetData().target = 34963;  // ELEMENT_ARRAY_BUFFER.
    const std::string data =
        "named_class:Tatooine"
        "named_class:Corusant"
        "named_class:Naboo"
        "named_class:Alderaan"
        "named_class:Dagobah"
        "named_class:Mandalore"
        "named_class:Corellia"
        "named_class:Kamino"
        "named_class:Kashyyyk"
        "named_class:Dantooine"
        "named_class:Hoth"
        "named_class:Mustafar"
        "named_class:Bespin"
        "named_class:Yavin"
        "named_class:Geonosis"
        "UNLABELED";
    property->GetData().data.assign(data.begin(), data.end());
    property->GetStringOffsets().type = "UINT32";
    property->GetStringOffsets().data.target = 34963;  // ELEMENT_ARRAY_BUFFER.
    property->GetStringOffsets().data.data = {0,   0, 0, 0,  // Tatooine
                                              20,  0, 0, 0,  // Corusant
                                              40,  0, 0, 0,  // Naboo
                                              57,  0, 0, 0,  // Alderaan
                                              77,  0, 0, 0,  // Dagobah
                                              96,  0, 0, 0,  // Mandalore
                                              117, 0, 0, 0,  // Corellia
                                              137, 0, 0, 0,  // Kamino
                                              155, 0, 0, 0,  // Kashyyyk
                                              175, 0, 0, 0,  // Dantooine
                                              196, 0, 0, 0,  // Hoth
                                              212, 0, 0, 0,  // Mustafar
                                              232, 0, 0, 0,  // Bespin
                                              250, 0, 0, 0,  // Yavin
                                              11,  1, 0, 0,  // Geonosis
                                              31,  1, 0, 0,  // UNLABELED
                                              40,  1, 0, 0};
    table->AddProperty(std::move(property));
  }

  // Add property that contains variable-length number sequence of the planet
  // class.
  {
    std::unique_ptr<PropertyTable::Property> property(
        new PropertyTable::Property());
    property->SetName("sequence");
    property->GetData().target = 34963;  // ELEMENT_ARRAY_BUFFER.
    const std::vector<float> data = {
        0.5f,  1.5f,  2.5f,  3.5f,  4.5f, 5.5f,  // Tatooine
        6.5f,  7.5f,                             // Corusant
        8.5f,                                    // Naboo
        9.5f,                                    // Alderaan
        10.5f, 11.5f,                            // Dagobah
        12.5f, 13.5f, 14.5f, 15.5f,              // Mandalore
        16.5f, 17.5f,                            // Corellia
        18.5f, 19.5f,                            // Kamino
        20.5f, 21.5f, 22.5f,                     // Kashyyyk
        23.5f, 24.5f, 25.5f,                     // Dantooine
        26.5f, 27.5f,                            // Hoth
        28.5f, 29.5f,                            // Mustafar
        30.5f, 31.5f, 32.5f,                     // Bespin
        33.5f, 34.5f, 35.5f,                     // Yavin
        36.5f, 37.5f, 38.5f, 39.5f, 40.5f        // Geonosis
    };                                           // UNLABELED (empty array).
    property->GetData().data.resize(4 * data.size());
    memcpy(property->GetData().data.data(), data.data(), 4 * data.size());
    property->GetArrayOffsets().type = "UINT8";
    property->GetArrayOffsets().data.target = 34963;  // ELEMENT_ARRAY_BUFFER.
    property->GetArrayOffsets().data.data = {
        0 * 4,   // Tatooine
        6 * 4,   // Corusant
        8 * 4,   // Naboo
        9 * 4,   // Alderaan
        10 * 4,  // Dagobah
        12 * 4,  // Mandalore
        16 * 4,  // Corellia
        18 * 4,  // Kamino
        20 * 4,  // Kashyyyk
        23 * 4,  // Dantooine
        26 * 4,  // Hoth
        28 * 4,  // Mustafar
        30 * 4,  // Bespin
        33 * 4,  // Yavin
        36 * 4,  // Geonosis
        41 * 4,  // UNLABELED (empty array).
        41 * 4};
    table->AddProperty(std::move(property));
  }

  // Add property table to the scene.
  scene->GetStructuralMetadata().AddPropertyTable(std::move(table));

  // Add structural metadata property attribute.
  std::unique_ptr<PropertyAttribute> attribute(new PropertyAttribute());
  attribute->SetName("The movement.");
  attribute->SetClass("movement");
  {
    std::unique_ptr<PropertyAttribute::Property> property(
        new PropertyAttribute::Property());
    property->SetName("direction");
    property->SetAttributeName("_DIRECTION");
    attribute->AddProperty(std::move(property));
  }
  {
    std::unique_ptr<PropertyAttribute::Property> property(
        new PropertyAttribute::Property());
    property->SetName("magnitude");
    property->SetAttributeName("_MAGNITUDE");
    attribute->AddProperty(std::move(property));
  }
  scene->GetStructuralMetadata().AddPropertyAttribute(std::move(attribute));

  // Get mesh element counts.
  Mesh &mesh = scene->GetMesh(MeshIndex(0));
  ASSERT_EQ(mesh.num_faces(), 12);
  ASSERT_EQ(mesh.num_points(), 36);
  const int num_vertices =
      mesh.GetNamedAttribute(GeometryAttribute::POSITION)->size();

  // Add per-vertex Float32 3D vector property attribute named _DIRECTION.
  {
    // Create property attribute.
    constexpr DataType kType = DataType::DT_FLOAT32;
    std::unique_ptr<PointAttribute> pa(new PointAttribute());
    pa->Init(GeometryAttribute::GENERIC, 3, kType, false, num_vertices);
    for (AttributeValueIndex avi(0); avi < num_vertices; ++avi) {
      const std::array<float, 3> val = {
          avi.value() + 0.10f, avi.value() + 0.20f, avi.value() + 0.30f};
      pa->SetAttributeValue(avi, &val);
    }
    const int att_id = mesh.AddPerVertexAttribute(std::move(pa));
    mesh.attribute(att_id)->set_name("_DIRECTION");
  }

  // Add per-vertex Float32 scalar property attribute named _MAGNITUDE.
  {
    // Create property attribute.
    constexpr DataType kType = DataType::DT_FLOAT32;
    std::unique_ptr<PointAttribute> pa(new PointAttribute());
    pa->Init(GeometryAttribute::GENERIC, 1, kType, false, num_vertices);
    for (AttributeValueIndex avi(0); avi < num_vertices; ++avi) {
      const float val = avi.value();
      pa->SetAttributeValue(avi, &val);
    }
    const int att_id = mesh.AddPerVertexAttribute(std::move(pa));
    mesh.attribute(att_id)->set_name("_MAGNITUDE");
  }

  // Add property attribute to the mesh.
  mesh.AddPropertyAttributesIndex(0);
}

template <>
void GltfTestHelper::CheckBoxMetaMeshFeatures(const Mesh &geometry,
                                              const UseCase &use_case) {
  CheckBoxMetaMeshFeatures(geometry, geometry.GetNonMaterialTextureLibrary(),
                           use_case);
}

template <>
void GltfTestHelper::CheckBoxMetaMeshFeatures(const Scene &geometry,
                                              const UseCase &use_case) {
  ASSERT_EQ(geometry.NumMeshes(), 1);
  CheckBoxMetaMeshFeatures(geometry.GetMesh(MeshIndex(0)),
                           geometry.GetNonMaterialTextureLibrary(), use_case);
}

void GltfTestHelper::CheckBoxMetaMeshFeatures(const Mesh &mesh,
                                              const TextureLibrary &texture_lib,
                                              const UseCase &use_case) {
  // Check texture library.
  ASSERT_EQ(texture_lib.NumTextures(), 2);

  // Check basic mesh properties.
  ASSERT_EQ(mesh.NumMeshFeatures(), 5);
  ASSERT_EQ(mesh.num_faces(), 12);
  ASSERT_EQ(mesh.num_attributes(), use_case.has_structural_metadata ? 9 : 7);
  ASSERT_EQ(mesh.num_points(), 36);
  ASSERT_EQ(mesh.NumNamedAttributes(GeometryAttribute::GENERIC),
            use_case.has_structural_metadata ? 5 : 3);
  ASSERT_EQ(mesh.NumNamedAttributes(GeometryAttribute::TEX_COORD), 2);

  // Get mesh element counts.
  const int num_faces = mesh.num_faces();
  const int num_corners = 3 * mesh.num_faces();
  const int num_vertices =
      mesh.GetNamedAttribute(GeometryAttribute::POSITION)->size();

  // Check mesh feature ID set at index 0.
  {
    // Check mesh features.
    const MeshFeatures &features = mesh.GetMeshFeatures(MeshFeaturesIndex(0));
    ASSERT_EQ(features.GetLabel(), "faces");
    ASSERT_EQ(features.GetFeatureCount(), num_faces);
    ASSERT_EQ(features.GetNullFeatureId(), 100);
    ASSERT_EQ(features.GetPropertyTableIndex(), 0);
    ASSERT_EQ(features.GetAttributeIndex(),
              use_case.has_structural_metadata ? 5 : 4);
    ASSERT_TRUE(features.GetTextureChannels().empty());
    ASSERT_EQ(features.GetTextureMap().texture(), nullptr);
    ASSERT_EQ(features.GetTextureMap().tex_coord_index(), -1);

    // Check per-face Uint8 attribute named _FEATURE_ID_0.
    const int att_id = features.GetAttributeIndex();
    const auto att = mesh.attribute(att_id);
    ASSERT_NE(att, nullptr);
    ASSERT_EQ(att->attribute_type(), GeometryAttribute::GENERIC);
    ASSERT_EQ(att->data_type(), DataType::DT_UINT8);
    ASSERT_EQ(att->num_components(), 1);
    ASSERT_EQ(att->size(), num_faces);
    ASSERT_EQ(att->indices_map_size(), num_corners);

    // Check that the values are all the numbers from 0 to 12.
    const std::vector<uint8_t> expected_values =
        use_case.has_draco_compression
            ? std::vector<uint8_t>{7, 11, 10, 3, 2, 5, 4, 1, 6, 9, 8, 0}
            : std::vector<uint8_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    for (int i = 0; i < num_faces; i++) {
      uint8_t val;
      att->GetValue(AttributeValueIndex(i), &val);
      ASSERT_EQ(val, expected_values[i]);
    }

    // Check that the corners of each face have a common value.
    for (int i = 0; i < num_faces; i++) {
      const auto face = mesh.face(FaceIndex(i));
      ASSERT_EQ(*att->GetAddressOfMappedIndex(face[0]),
                *att->GetAddressOfMappedIndex(face[1]));
      ASSERT_EQ(*att->GetAddressOfMappedIndex(face[0]),
                *att->GetAddressOfMappedIndex(face[2]));
    }
  }

  // Check the 2nd mesh feature ID set at index 1.
  {
    // Check mesh features.
    const MeshFeatures &features = mesh.GetMeshFeatures(MeshFeaturesIndex(1));
    ASSERT_EQ(features.GetLabel(), "vertices");
    ASSERT_EQ(features.GetFeatureCount(), num_vertices);
    ASSERT_EQ(features.GetNullFeatureId(), 101);
    ASSERT_EQ(features.GetPropertyTableIndex(), 1);
    ASSERT_EQ(features.GetAttributeIndex(),
              use_case.has_structural_metadata ? 6 : 5);
    ASSERT_TRUE(features.GetTextureChannels().empty());
    ASSERT_EQ(features.GetTextureMap().texture(), nullptr);
    ASSERT_EQ(features.GetTextureMap().tex_coord_index(), -1);

    // Check per-vertex Uint16 attribute named _FEATURE_ID_1.
    const int att_id = features.GetAttributeIndex();
    const auto att = mesh.attribute(att_id);
    ASSERT_NE(att, nullptr);
    ASSERT_EQ(att->attribute_type(), GeometryAttribute::GENERIC);
    ASSERT_EQ(att->data_type(), DataType::DT_UINT16);
    ASSERT_EQ(att->num_components(), 1);
    ASSERT_EQ(att->size(), num_vertices);
    ASSERT_EQ(att->indices_map_size(), num_corners);

    // Check that the values are all the numbers from 0 to 7.
    const std::vector<uint16_t> expected_values =
        use_case.has_draco_compression
            ? std::vector<uint16_t>{3, 6, 7, 4, 5, 0, 1, 2}
            : std::vector<uint16_t>{0, 1, 2, 3, 4, 5, 6, 7};
    for (int i = 0; i < num_vertices; i++) {
      uint16_t val;
      att->GetValue(AttributeValueIndex(i), &val);
      ASSERT_EQ(val, expected_values[i]);
    }

    // Check that the corners of a face have unique values.
    for (int i = 0; i < num_faces; i++) {
      const auto face = mesh.face(FaceIndex(i));
      ASSERT_NE(*att->GetAddressOfMappedIndex(face[0]),
                *att->GetAddressOfMappedIndex(face[1]));
      ASSERT_NE(*att->GetAddressOfMappedIndex(face[1]),
                *att->GetAddressOfMappedIndex(face[2]));
      ASSERT_NE(*att->GetAddressOfMappedIndex(face[2]),
                *att->GetAddressOfMappedIndex(face[0]));
    }
  }

  // Check the 3rd mesh feature ID set at index 2.
  {
    // Check mesh features.
    const MeshFeatures &features = mesh.GetMeshFeatures(MeshFeaturesIndex(2));
    ASSERT_TRUE(features.GetLabel().empty());
    ASSERT_EQ(features.GetFeatureCount(), num_corners);
    ASSERT_EQ(features.GetNullFeatureId(), -1);
    ASSERT_EQ(features.GetPropertyTableIndex(), -1);
    ASSERT_EQ(features.GetAttributeIndex(),
              use_case.has_structural_metadata ? 7 : 6);
    ASSERT_TRUE(features.GetTextureChannels().empty());
    ASSERT_EQ(features.GetTextureMap().texture(), nullptr);
    ASSERT_EQ(features.GetTextureMap().tex_coord_index(), -1);

    // Check per-corner Float attribute named _FEATURE_ID_2.
    const int att_id = features.GetAttributeIndex();
    const auto att = mesh.attribute(att_id);
    ASSERT_NE(att, nullptr);
    ASSERT_EQ(att->attribute_type(), GeometryAttribute::GENERIC);
    ASSERT_EQ(att->data_type(), DataType::DT_FLOAT32);
    ASSERT_EQ(att->num_components(), 1);
    ASSERT_EQ(att->size(), num_corners);
    ASSERT_EQ(att->indices_map_size(), 0);
    ASSERT_TRUE(att->is_mapping_identity());

    // Check that the values are from 0 to 35.
    const std::vector<float> expected_values =
        use_case.has_draco_compression
            ? std::vector<float>{23, 21, 22, 33, 34, 35, 31, 32, 30, 9, 10, 11,
                                 7,  8,  6,  15, 16, 17, 14, 12, 13, 5, 3,  4,
                                 19, 20, 18, 27, 28, 29, 26, 24, 25, 1, 2,  0}
            : std::vector<float>{0,  1,  2,  3,  4,  5,  6,  7,  8,
                                 9,  10, 11, 12, 13, 14, 15, 16, 17,
                                 18, 19, 20, 21, 22, 23, 24, 25, 26,
                                 27, 28, 29, 30, 31, 32, 33, 34, 35};
    for (int i = 0; i < num_corners; i++) {
      float val;
      att->GetValue(AttributeValueIndex(i), &val);
      ASSERT_EQ(val, expected_values[i]);
    }

    // Check that the corners have unique values.
    for (int i = 0; i < num_faces; i++) {
      const auto face = mesh.face(FaceIndex(i));
      float v0, v1, v2;
      att->GetMappedValue(face[0], &v0);
      att->GetMappedValue(face[1], &v1);
      att->GetMappedValue(face[2], &v2);
      ASSERT_EQ(v0, expected_values[3 * i + 0]);
      ASSERT_EQ(v1, expected_values[3 * i + 1]);
      ASSERT_EQ(v2, expected_values[3 * i + 2]);
    }
  }

  // Check mesh feature ID set at index 3.
  {
    // Check mesh features.
    const MeshFeatures &features = mesh.GetMeshFeatures(MeshFeaturesIndex(3));
    ASSERT_TRUE(features.GetLabel().empty());
    ASSERT_EQ(features.GetFeatureCount(), 6);
    ASSERT_EQ(features.GetNullFeatureId(), -1);
    ASSERT_EQ(features.GetPropertyTableIndex(), -1);
    ASSERT_EQ(features.GetAttributeIndex(), -1);
  }

  // Check mesh feature ID set at index 4.
  {
    // Check mesh features.
    const MeshFeatures &features = mesh.GetMeshFeatures(MeshFeaturesIndex(4));
    ASSERT_EQ(features.GetLabel(), "water");
    ASSERT_EQ(features.GetFeatureCount(), 2);
    ASSERT_EQ(features.GetNullFeatureId(), -1);
    ASSERT_EQ(features.GetPropertyTableIndex(), -1);
    ASSERT_EQ(features.GetAttributeIndex(), -1);
  }
}

void GltfTestHelper::CheckBoxMetaStructuralMetadata(
    const Mesh &mesh, const StructuralMetadata &structural_metadata,
    const UseCase &use_case) {
  // Check structural metadata schema.
  {
    const StructuralMetadataSchema &schema = structural_metadata.GetSchema();
    ASSERT_FALSE(schema.Empty());
    const StructuralMetadataSchema::Object &json = schema.json;
    ASSERT_EQ(json.GetObjects().size(), 3);
    ASSERT_EQ(json.GetObjects()[0].GetName(), "classes");
    ASSERT_EQ(json.GetObjects()[0].GetObjects().size(), 2);

    // Check class "movement".
    {
      const auto item = json.GetObjects()[0].GetObjects()[0];
      ASSERT_EQ(item.GetName(), "movement");
      ASSERT_EQ(item.GetObjects().size(), 3);

      const auto &description = item.GetObjects()[0];
      ASSERT_EQ(description.GetName(), "description");
      ASSERT_EQ(description.GetString(), "Vertex movement.");

      const auto &name = item.GetObjects()[1];
      ASSERT_EQ(name.GetName(), "name");
      ASSERT_EQ(name.GetString(), "The movement.");

      const auto &properties = item.GetObjects()[2];
      ASSERT_EQ(properties.GetName(), "properties");
      ASSERT_EQ(properties.GetObjects().size(), 2);

      const auto &direction = properties.GetObjects()[0];
      ASSERT_EQ(direction.GetName(), "direction");
      ASSERT_EQ(direction.GetObjects().size(), 4);
      ASSERT_EQ(direction.GetObjects()[0].GetName(), "componentType");
      ASSERT_EQ(direction.GetObjects()[1].GetName(), "description");
      ASSERT_EQ(direction.GetObjects()[2].GetName(), "required");
      ASSERT_EQ(direction.GetObjects()[3].GetName(), "type");
      ASSERT_EQ(direction.GetObjects()[0].GetString(), "FLOAT32");
      ASSERT_EQ(direction.GetObjects()[1].GetString(), "Movement direction.");
      ASSERT_EQ(direction.GetObjects()[2].GetBoolean(), true);
      ASSERT_EQ(direction.GetObjects()[3].GetString(), "VEC3");

      const auto &mag = properties.GetObjects()[1];
      ASSERT_EQ(mag.GetName(), "magnitude");
      ASSERT_EQ(mag.GetObjects().size(), 4);
      ASSERT_EQ(mag.GetObjects()[0].GetName(), "componentType");
      ASSERT_EQ(mag.GetObjects()[1].GetName(), "description");
      ASSERT_EQ(mag.GetObjects()[2].GetName(), "required");
      ASSERT_EQ(mag.GetObjects()[3].GetName(), "type");
      ASSERT_EQ(mag.GetObjects()[0].GetString(), "FLOAT32");
      ASSERT_EQ(mag.GetObjects()[1].GetString(), "Movement magnitude.");
      ASSERT_EQ(mag.GetObjects()[2].GetBoolean(), true);
      ASSERT_EQ(mag.GetObjects()[3].GetString(), "SCALAR");
    }

    // Check class "planet".
    {
      const auto item = json.GetObjects()[0].GetObjects()[1];
      ASSERT_EQ(item.GetName(), "planet");
      ASSERT_EQ(item.GetObjects().size(), 1);

      const auto &properties = item.GetObjects()[0];
      ASSERT_EQ(properties.GetName(), "properties");
      ASSERT_EQ(properties.GetObjects().size(), 3);

      const auto &color = properties.GetObjects()[0];
      ASSERT_EQ(color.GetName(), "color");
      ASSERT_EQ(color.GetObjects().size(), 4);
      ASSERT_EQ(color.GetObjects()[0].GetName(), "componentType");
      ASSERT_EQ(color.GetObjects()[1].GetName(), "description");
      ASSERT_EQ(color.GetObjects()[2].GetName(), "required");
      ASSERT_EQ(color.GetObjects()[3].GetName(), "type");
      ASSERT_EQ(color.GetObjects()[0].GetString(), "UINT8");
      ASSERT_EQ(color.GetObjects()[1].GetString(), "The RGB color.");
      ASSERT_TRUE(color.GetObjects()[2].GetBoolean());
      ASSERT_EQ(color.GetObjects()[3].GetString(), "VEC3");

      const auto &name = properties.GetObjects()[1];
      ASSERT_EQ(name.GetName(), "name");
      ASSERT_EQ(name.GetObjects().size(), 3);
      ASSERT_EQ(name.GetObjects()[0].GetName(), "description");
      ASSERT_EQ(name.GetObjects()[1].GetName(), "required");
      ASSERT_EQ(name.GetObjects()[2].GetName(), "type");
      ASSERT_EQ(name.GetObjects()[0].GetString(), "The name.");
      ASSERT_TRUE(name.GetObjects()[1].GetBoolean());
      ASSERT_EQ(name.GetObjects()[2].GetString(), "STRING");

      const auto &sequence = properties.GetObjects()[2];
      ASSERT_EQ(sequence.GetName(), "sequence");
      ASSERT_EQ(sequence.GetObjects().size(), 4);
      ASSERT_EQ(sequence.GetObjects()[0].GetName(), "componentType");
      ASSERT_EQ(sequence.GetObjects()[1].GetName(), "description");
      ASSERT_EQ(sequence.GetObjects()[2].GetName(), "required");
      ASSERT_EQ(sequence.GetObjects()[3].GetName(), "type");
      ASSERT_EQ(sequence.GetObjects()[0].GetString(), "FLOAT32");
      ASSERT_EQ(sequence.GetObjects()[1].GetString(), "The number sequence.");
      ASSERT_FALSE(sequence.GetObjects()[2].GetBoolean());
      ASSERT_EQ(sequence.GetObjects()[3].GetString(), "SCALAR");
    }

    ASSERT_EQ(json.GetObjects()[1].GetName(), "enums");
    const auto &classifications = json.GetObjects()[1].GetObjects()[0];
    ASSERT_EQ(classifications.GetName(), "classifications");
    ASSERT_EQ(classifications.GetObjects()[0].GetName(), "description");
    ASSERT_EQ(classifications.GetObjects()[0].GetString(),
              "Classifications of planets.");
    ASSERT_EQ(classifications.GetObjects()[1].GetName(), "name");
    ASSERT_EQ(classifications.GetObjects()[1].GetString(), "classifications");
    ASSERT_EQ(classifications.GetObjects()[2].GetName(), "values");
    const auto &values = classifications.GetObjects()[2];
    ASSERT_EQ(values.GetArray()[0].GetObjects()[0].GetName(), "name");
    ASSERT_EQ(values.GetArray()[1].GetObjects()[0].GetName(), "name");
    ASSERT_EQ(values.GetArray()[2].GetObjects()[0].GetName(), "name");
    ASSERT_EQ(values.GetArray()[3].GetObjects()[0].GetName(), "name");
    ASSERT_EQ(values.GetArray()[4].GetObjects()[0].GetName(), "name");
    ASSERT_EQ(values.GetArray()[0].GetObjects()[0].GetString(), "Unspecified");
    ASSERT_EQ(values.GetArray()[1].GetObjects()[0].GetString(), "Gas Giant");
    ASSERT_EQ(values.GetArray()[2].GetObjects()[0].GetString(), "Waterworld");
    ASSERT_EQ(values.GetArray()[3].GetObjects()[0].GetString(), "Agriworld");
    ASSERT_EQ(values.GetArray()[4].GetObjects()[0].GetString(), "Ordnance");
    ASSERT_EQ(values.GetArray()[0].GetObjects()[1].GetName(), "value");
    ASSERT_EQ(values.GetArray()[1].GetObjects()[1].GetName(), "value");
    ASSERT_EQ(values.GetArray()[2].GetObjects()[1].GetName(), "value");
    ASSERT_EQ(values.GetArray()[3].GetObjects()[1].GetName(), "value");
    ASSERT_EQ(values.GetArray()[4].GetObjects()[1].GetName(), "value");
    ASSERT_EQ(values.GetArray()[0].GetObjects()[1].GetInteger(), 0);
    ASSERT_EQ(values.GetArray()[1].GetObjects()[1].GetInteger(), 1);
    ASSERT_EQ(values.GetArray()[2].GetObjects()[1].GetInteger(), 2);
    ASSERT_EQ(values.GetArray()[3].GetObjects()[1].GetInteger(), 3);
    ASSERT_EQ(values.GetArray()[4].GetObjects()[1].GetInteger(), 4);

    ASSERT_EQ(json.GetObjects()[2].GetName(), "id");
    ASSERT_EQ(json.GetObjects()[2].GetString(), "galaxy");
  }

  // Check property table.
  constexpr int kRows = 16;
  ASSERT_EQ(structural_metadata.NumPropertyTables(), 1);
  const PropertyTable &table = structural_metadata.GetPropertyTable(0);
  ASSERT_EQ(table.GetName(), "Galaxy far far away.");
  ASSERT_EQ(table.GetClass(), "planet");
  ASSERT_EQ(table.GetCount(), kRows);
  ASSERT_EQ(table.NumProperties(), 3);

  // Check property that describes RGB color components of the planet class.
  {
    const PropertyTable::Property &property = table.GetProperty(0);
    ASSERT_EQ(property.GetName(), "color");

    ASSERT_EQ(property.GetData().data.size(), kRows * 3);  // RGB components.
    ASSERT_EQ(property.GetData().target, 34962);           // ARRAY_BUFFER.

    ASSERT_EQ(property.GetData().data[0], 94);  // Tatooine [94,  94,  194].
    ASSERT_EQ(property.GetData().data[1], 94);
    ASSERT_EQ(property.GetData().data[2], 194);
    ASSERT_EQ(property.GetData().data[18], 190);  // Corellia [190, 92,  108].
    ASSERT_EQ(property.GetData().data[19], 92);
    ASSERT_EQ(property.GetData().data[20], 108);
    ASSERT_EQ(property.GetData().data[45], 0);  // UNLABELED [0, 0, 0].
    ASSERT_EQ(property.GetData().data[46], 0);
    ASSERT_EQ(property.GetData().data[47], 0);

    ASSERT_TRUE(property.GetArrayOffsets().type.empty());
    ASSERT_TRUE(property.GetArrayOffsets().data.data.empty());
    ASSERT_EQ(property.GetArrayOffsets().data.target, 0);
    ASSERT_TRUE(property.GetStringOffsets().type.empty());
    ASSERT_TRUE(property.GetStringOffsets().data.data.empty());
    ASSERT_EQ(property.GetStringOffsets().data.target, 0);
  }

  // Check property that describes names of the planet class.
  {
    const PropertyTable::Property &property = table.GetProperty(1);
    ASSERT_EQ(property.GetName(), "name");
    const std::vector<uint8_t> &data = property.GetData().data;
    const std::vector<uint8_t> &offsets = property.GetStringOffsets().data.data;

    ASSERT_EQ(data.size(), 296);                  // Concatenated label strings.
    ASSERT_EQ(property.GetData().target, 34963);  // ELEMENT_ARRAY_BUFFER.

    ASSERT_EQ(property.GetStringOffsets().type, "UINT32");
    ASSERT_EQ(offsets.size(), 4 * (kRows + 1));
    ASSERT_EQ(property.GetStringOffsets().data.target, 34963);

    ASSERT_EQ(offsets[0], 0);  // Tatooine 0.
    ASSERT_EQ(offsets[1], 0);
    ASSERT_EQ(offsets[2], 0);
    ASSERT_EQ(offsets[3], 0);
    ASSERT_EQ(offsets[60], 31);  // UNLABELED 287.
    ASSERT_EQ(offsets[61], 1);
    ASSERT_EQ(offsets[62], 0);
    ASSERT_EQ(offsets[63], 0);
    ASSERT_EQ(offsets[64], 40);  // Beyond UNLABELED 296.
    ASSERT_EQ(offsets[65], 1);
    ASSERT_EQ(offsets[66], 0);
    ASSERT_EQ(offsets[67], 0);

    struct Name {
      static std::string Extract(const std::vector<uint8_t> &data,
                                 const std::vector<uint8_t> &offsets, int row) {
        const int b = offsets[4 * (row + 0)] + 256 * offsets[4 * (row + 0) + 1];
        const int e = offsets[4 * (row + 1)] + 256 * offsets[4 * (row + 1) + 1];
        return std::string(data.begin() + b, data.begin() + e);
      }
    };

    // Check that the names can be extracted from the data.
    ASSERT_EQ(Name::Extract(data, offsets, 0), "named_class:Tatooine");
    ASSERT_EQ(Name::Extract(data, offsets, 6), "named_class:Corellia");
    ASSERT_EQ(Name::Extract(data, offsets, 12), "named_class:Bespin");
    ASSERT_EQ(Name::Extract(data, offsets, 13), "named_class:Yavin");
    ASSERT_EQ(Name::Extract(data, offsets, 14), "named_class:Geonosis");
    ASSERT_EQ(Name::Extract(data, offsets, 15), "UNLABELED");

    ASSERT_TRUE(property.GetArrayOffsets().type.empty());
    ASSERT_TRUE(property.GetArrayOffsets().data.data.empty());
    ASSERT_EQ(property.GetArrayOffsets().data.target, 0);
  }

  // Check property that describes number sequence of the planet class.
  {
    const PropertyTable::Property &property = table.GetProperty(2);
    ASSERT_EQ(property.GetName(), "sequence");
    const std::vector<uint8_t> &data = property.GetData().data;
    const std::vector<uint8_t> &offsets = property.GetArrayOffsets().data.data;

    ASSERT_EQ(data.size(), 41 * 4);               // Concatenated float arrays.
    ASSERT_EQ(property.GetData().target, 34963);  // ELEMENT_ARRAY_BUFFER.

    ASSERT_EQ(property.GetArrayOffsets().type, "UINT8");
    ASSERT_EQ(offsets.size(), 20);  // kRows + 1 + padding.
    ASSERT_EQ(property.GetArrayOffsets().data.target, 34963);

    ASSERT_EQ(offsets[0], 0 * 4);    // Tatooine
    ASSERT_EQ(offsets[1], 6 * 4);    // Corusant
    ASSERT_EQ(offsets[6], 16 * 4);   // Corellia
    ASSERT_EQ(offsets[14], 36 * 4);  // Geonosis
    ASSERT_EQ(offsets[15], 41 * 4);  // UNLABELED (empty array).
    ASSERT_EQ(offsets[16], 41 * 4);  // Beyond UNLABELED (empty array).

    struct Sequence {
      static std::vector<float> Extract(const std::vector<uint8_t> &data,
                                        const std::vector<uint8_t> &offsets,
                                        int row) {
        const int n = (offsets[row + 1] - offsets[row]) / 4;
        std::vector<float> result;
        result.reserve(n);
        for (int i = 0; i < n; ++i) {
          const void *const pointer = &data[offsets[row] + 4 * i];
          result.push_back(*static_cast<const float *>(pointer));
        }
        return result;
      }
    };

    // Check that the number sequence arrays can be extracted from the data.
    ASSERT_EQ(
        Sequence::Extract(data, offsets, 0),
        (std::vector<float>{0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f}));  // Tatooine
    ASSERT_EQ(Sequence::Extract(data, offsets, 1),
              (std::vector<float>{6.5f, 7.5f}));  // Corusant
    ASSERT_EQ(
        Sequence::Extract(data, offsets, 14),
        (std::vector<float>{36.5f, 37.5f, 38.5f, 39.5f, 40.5f}));  // Geonosis
    ASSERT_TRUE(Sequence::Extract(data, offsets, 15)
                    .empty());  // UNLABELED (empty array).

    ASSERT_TRUE(property.GetStringOffsets().type.empty());
    ASSERT_TRUE(property.GetStringOffsets().data.data.empty());
    ASSERT_EQ(property.GetStringOffsets().data.target, 0);
  }

  // Check property attributes in structural metadata.
  ASSERT_EQ(structural_metadata.NumPropertyAttributes(), 1);
  {
    const PropertyAttribute &attribute =
        structural_metadata.GetPropertyAttribute(0);
    ASSERT_EQ(attribute.GetName(), "The movement.");
    ASSERT_EQ(attribute.GetClass(), "movement");
    ASSERT_EQ(attribute.NumProperties(), 2);

    const PropertyAttribute::Property &direction = attribute.GetProperty(0);
    ASSERT_EQ(direction.GetName(), "direction");
    ASSERT_EQ(direction.GetAttributeName(), "_DIRECTION");

    const PropertyAttribute::Property &magnitude = attribute.GetProperty(1);
    ASSERT_EQ(magnitude.GetName(), "magnitude");
    ASSERT_EQ(magnitude.GetAttributeName(), "_MAGNITUDE");
  }

  // Check property attributes in the |mesh|.
  ASSERT_EQ(mesh.NumPropertyAttributesIndices(), 1);
  ASSERT_EQ(mesh.GetPropertyAttributesIndex(0), 0);
  ASSERT_EQ(mesh.num_faces(), 12);
  ASSERT_EQ(mesh.num_attributes(), 9);
  ASSERT_EQ(mesh.num_points(), 36);
  ASSERT_EQ(mesh.NumNamedAttributes(GeometryAttribute::GENERIC), 5);

  // Get mesh element counts.
  const int num_corners = 3 * mesh.num_faces();
  const int num_vertices =
      mesh.GetNamedAttribute(GeometryAttribute::POSITION)->size();

  // Check property attribute named _DIRECTION.
  {
    const auto att =
        mesh.GetNamedAttributeByName(GeometryAttribute::GENERIC, "_DIRECTION");
    ASSERT_NE(att, nullptr);
    ASSERT_EQ(att->attribute_type(), GeometryAttribute::GENERIC);
    ASSERT_EQ(att->data_type(), DataType::DT_FLOAT32);
    ASSERT_EQ(att->num_components(), 3);
    ASSERT_EQ(att->size(), num_vertices);
    ASSERT_EQ(att->indices_map_size(), num_corners);

    // Check attribute values.
    // clang-format off
    const std::vector<float> expected_values =
        use_case.has_draco_compression
            ? std::vector<float>{3.1f, 3.2f, 3.3f,
                                 6.1f, 6.2f, 6.3f,
                                 7.1f, 7.2f, 7.3f,
                                 4.1f, 4.2f, 4.3f,
                                 5.1f, 5.2f, 5.3f,
                                 0.1f, 0.2f, 0.3f,
                                 1.1f, 1.2f, 1.3f,
                                 2.1f, 2.2f, 2.3f}
            : std::vector<float>{0.1f, 0.2f, 0.3f,
                                 1.1f, 1.2f, 1.3f,
                                 2.1f, 2.2f, 2.3f,
                                 3.1f, 3.2f, 3.3f,
                                 4.1f, 4.2f, 4.3f,
                                 5.1f, 5.2f, 5.3f,
                                 6.1f, 6.2f, 6.3f,
                                 7.1f, 7.2f, 7.3f};
    // clang-format on
    for (int i = 0; i < num_vertices; i++) {
      std::array<float, 3> val;
      att->GetValue(AttributeValueIndex(i), &val);
      ASSERT_EQ(val[0], expected_values[3 * i + 0]);
      ASSERT_EQ(val[1], expected_values[3 * i + 1]);
      ASSERT_EQ(val[2], expected_values[3 * i + 2]);
    }
  }

  // Check property attribute named _MAGNITUDE.
  {
    const auto att =
        mesh.GetNamedAttributeByName(GeometryAttribute::GENERIC, "_MAGNITUDE");
    ASSERT_NE(att, nullptr);
    ASSERT_EQ(att->attribute_type(), GeometryAttribute::GENERIC);
    ASSERT_EQ(att->data_type(), DataType::DT_FLOAT32);
    ASSERT_EQ(att->num_components(), 1);
    ASSERT_EQ(att->size(), num_vertices);
    ASSERT_EQ(att->indices_map_size(), num_corners);

    // Check attribute values.
    const std::vector<float> expected_values =
        use_case.has_draco_compression
            ? std::vector<float>{3.f, 6.f, 7.f, 4.f, 5.f, 0.f, 1.f, 2.f}
            : std::vector<float>{0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f};
    for (int i = 0; i < num_vertices; i++) {
      float val;
      att->GetValue(AttributeValueIndex(i), &val);
      ASSERT_EQ(val, expected_values[i]);
    }
  }
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace draco
