#include "VizEngineAPIs.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#undef GetObject

using namespace rapidjson;
namespace settingsIO {

// JSON에서 Material 값을 가져오는 재귀 함수
void ImportMaterials(const rapidjson::Value& jsonNode,
                     vzm::VzSceneComp* component) {
  vzm::SCENE_COMPONENT_TYPE type = component->GetSceneCompType();

  if (jsonNode.HasMember("name")) {
    //component->SetName(jsonNode["name"].GetString());
  }

  switch (type) {
    case vzm::SCENE_COMPONENT_TYPE::ACTOR: {
      vzm::VzActor* actor = (vzm::VzActor*)component;
      std::vector<VID> mis = actor->GetMIs();

      if (jsonNode.HasMember("materials")) {
        const rapidjson::Value& materials = jsonNode["materials"];

        for (rapidjson::SizeType i = 0; i < materials.Size(); ++i) {
          vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(mis[i]);
          vzm::VzMaterial* ma =
              (vzm::VzMaterial*)vzm::GetVzComponent(mi->GetMaterial());
          std::map<std::string, vzm::VzMaterial::ParameterInfo> pram;
          ma->GetAllowedParameters(pram);

          materials[i]["name"].GetString();

          const rapidjson::Value& parameters = materials[i]["parameters"];

          auto iter = pram.begin();

          for (rapidjson::SizeType j = 0; j < parameters.Size(); ++j) {
            std::string name = parameters[j]["name"].GetString();
            vzm::UniformType type =
                (vzm::UniformType)parameters[j]["type"].GetInt();
            bool isSampler = parameters[j]["isSampler"].GetBool();

            const rapidjson::Value& values = parameters[j]["value"];
            if (values.IsArray()) {
              std::vector<float> v;
              switch (type) {
                case vzm::UniformType::FLOAT3:
                  v.resize(3);
                  break;
                case vzm::UniformType::FLOAT4:
                  v.resize(4);
                  break;
                default: {
                  std::cerr << "type(array) error" << std::endl;
                  break;
                }
              }

              for (rapidjson::SizeType k = 0; k < values.Size(); ++k) {
                v[k] = values[k].GetFloat();
              }
              mi->SetParameter(name, type, (void*)v.data());
            } else {
              switch (type) {
                case vzm::UniformType::BOOL:
                  if (isSampler) {
                  } else {
                    bool value = values.GetBool();
                    mi->SetParameter(name, type, &value);
                  }
                  break;
                case vzm::UniformType::FLOAT: {
                  float value = values.GetFloat();
                  mi->SetParameter(name, type, &value);
                  break;
                }
                default: {
                  std::cerr << "type error" << std::endl;
                  break;
                }
              }
            }

            if (iter++ == pram.end()) {
              std::cerr << "pram error" << std::endl;
            }
          }
        }
      }     
    break;
  }
    case vzm::SCENE_COMPONENT_TYPE::CAMERA:
    {
      break;
    }
    case vzm::SCENE_COMPONENT_TYPE::LIGHT:
    {
      break;
    }
  }
  if (jsonNode.HasMember("children")) {
    const rapidjson::Value& children = jsonNode["children"];
    std::vector<VID> childrenVIDs = component->GetChildren();
    for (int i = 0; i < childrenVIDs.size(); ++i) {
      vzm::VzSceneComp* childComponent =
          (vzm::VzSceneComp*)vzm::GetVzComponent(childrenVIDs[i]);
      std::string childName = childComponent->GetName();
      ImportMaterials(children[childName.c_str()], childComponent);
    }
  }
}

// Material 값을 JSON으로 내보내는 재귀 함수
void ExportMaterials(rapidjson::Value& jsonNode,
                     rapidjson::Document::AllocatorType& allocator,
                     const VID nodeVID) {
  vzm::VzSceneComp* component = (vzm::VzSceneComp*)vzm::GetVzComponent(nodeVID);
  vzm::SCENE_COMPONENT_TYPE type = component->GetSceneCompType();

  jsonNode.SetObject();
  jsonNode.AddMember("name",
                     rapidjson::Value(component->GetName().c_str(), allocator),
                     allocator);

  switch (type) {
    case vzm::SCENE_COMPONENT_TYPE::ACTOR: {
      rapidjson::Value materials(rapidjson::kArrayType);
      vzm::VzActor* actor = (vzm::VzActor*)component;
      std::vector<VID> mis = actor->GetMIs();

      for (int mIdx = 0; mIdx < mis.size(); mIdx++) {
        vzm::VzMI* mi = (vzm::VzMI*)vzm::GetVzComponent(mis[mIdx]);

        rapidjson::Value materialObj(rapidjson::kObjectType);
        materialObj.AddMember(
            "name", rapidjson::Value(mi->GetName().c_str(), allocator),
            allocator);

        rapidjson::Value parameters(rapidjson::kArrayType);
        vzm::VzMaterial* ma =
            (vzm::VzMaterial*)vzm::GetVzComponent(mi->GetMaterial());
        std::map<std::string, vzm::VzMaterial::ParameterInfo> pram;
        ma->GetAllowedParameters(pram);
        // DoubleSided

        // TransparencyMode

        for (auto iter = pram.begin(); iter != pram.end(); iter++) {
          vzm::VzMaterial::ParameterInfo& paramInfo = iter->second;

          rapidjson::Value paramObj(rapidjson::kObjectType);
          paramObj.AddMember(
              "name", rapidjson::Value(paramInfo.name, allocator), allocator);
          paramObj.AddMember("type", (int)paramInfo.type, allocator);
          paramObj.AddMember("isSampler", paramInfo.isSampler, allocator);

          // bool, float, int 등
          // 현재는 bool, float만 있다고 가정
          if (!paramInfo.isSampler) {
            switch (paramInfo.type) {
              case vzm::UniformType::BOOL: {
                bool value;
                mi->GetParameter(paramInfo.name, paramInfo.type, &value);
                paramObj.AddMember("value", value, allocator);
                break;
              }
              case vzm::UniformType::FLOAT: {
                float value;
                mi->GetParameter(paramInfo.name, paramInfo.type, &value);
                paramObj.AddMember("value", value, allocator);
                break;
              }
              case vzm::UniformType::FLOAT2: {
                float value[2];
                mi->GetParameter(paramInfo.name, paramInfo.type, value);
                rapidjson::Value valueArray(rapidjson::kArrayType);
                for (float val : value) {
                  valueArray.PushBack(val, allocator);
                }
                paramObj.AddMember("value", valueArray, allocator);
                break;
              }
              case vzm::UniformType::FLOAT3: {
                float value[3];
                mi->GetParameter(paramInfo.name, paramInfo.type, value);
                rapidjson::Value valueArray(rapidjson::kArrayType);
                for (float val : value) {
                  valueArray.PushBack(val, allocator);
                }
                paramObj.AddMember("value", valueArray, allocator);
                break;
              }
              case vzm::UniformType::FLOAT4: {
                float value[4];
                mi->GetParameter(paramInfo.name, paramInfo.type, value);
                rapidjson::Value valueArray(rapidjson::kArrayType);
                for (float val : value) {
                  valueArray.PushBack(val, allocator);
                }
                paramObj.AddMember("value", valueArray, allocator);
                break;
              }
            }
          }
          // string(texture 경로)
          else {
            paramObj.AddMember("value",
                               rapidjson::Value("need_texture_path", allocator),
                               allocator);
          }

          rapidjson::Value paramName(paramInfo.name, allocator);
          parameters.PushBack(paramObj, allocator);
        }

        materialObj.AddMember("parameters", parameters, allocator);
        materials.PushBack(materialObj, allocator);
      }

      jsonNode.AddMember("materials", materials, allocator);
      break;  
  }
    case vzm::SCENE_COMPONENT_TYPE::CAMERA: {
      break;
    }
    case vzm::SCENE_COMPONENT_TYPE::LIGHT: {
      break;
    }
  }

  std::vector<VID> childrenVIDs = component->GetChildren();

  rapidjson::Value children;
  children.SetObject();
  for (const VID childVID : childrenVIDs) {
    rapidjson::Value childJsonNode;
    ExportMaterials(childJsonNode, allocator, childVID);

    std::string childName = childJsonNode["name"].GetString();
    children.AddMember(childJsonNode["name"], childJsonNode, allocator);
    childJsonNode["name"] = rapidjson::Value(childName.c_str(), allocator);
  }
  jsonNode.AddMember("children", children, allocator);
}

void importSettings(VID root, std::string filePath) {
  FILE* fp;
  fopen_s(&fp, filePath.c_str(), "r");
  if (fp == nullptr) return;

  char readBuffer[65536];
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  rapidjson::Document document;
  document.ParseStream(is);
  fclose(fp);

  vzm::VzSceneComp* component = (vzm::VzSceneComp*)vzm::GetVzComponent(root);
  ImportMaterials(document, component);
}

void exportSettings(VID root) {
  FILE* outfp;
  rapidjson::Document outputDoc;

  outputDoc.SetObject();
  ExportMaterials(outputDoc, outputDoc.GetAllocator(), root);
  fopen_s(&outfp, "settings.json", "w");
  if (outfp == nullptr) return;

  char writeBuffer[65536];
  rapidjson::FileWriteStream os(outfp, writeBuffer, sizeof(writeBuffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
  outputDoc.Accept(writer);
  fclose(outfp);

}

}  // namespace settingsIO
