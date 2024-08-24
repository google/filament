#include "savefileio.h"

#include <time.h>

#include <iostream>
#undef GetObject

using namespace rapidjson;
namespace savefileIO {
std::string res_path = "";

std::string getRelativePath(std::string absolute_path) {
  if (res_path.size() == 0) {
    exit(-1);
  }
  if (absolute_path.substr(0, res_path.length()) == res_path) {
    return absolute_path.substr(res_path.size(),
                                absolute_path.size() - res_path.size());
  } else {
    return "";
  }
}

void setResPath(std::string assetPath) {
  size_t offset = assetPath.find_last_of('\\');
  res_path = assetPath.substr(0, offset + 1);
}

// JSON에서 Material 값을 가져오는 재귀 함수
void ImportMaterials(const rapidjson::Value& jsonNode,
                     vzm::VzSceneComp* component) {
  vzm::SCENE_COMPONENT_TYPE type = component->GetSceneCompType();

  if (jsonNode.HasMember("name")) {
    // component->SetName(jsonNode["name"].GetString());
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

          std::string matName = materials[i]["name"].GetString();

          const rapidjson::Value& parameters = materials[i]["parameters"];

          auto iter = pram.begin();

          // DoubleSided
          mi->SetDoubleSided(parameters[0]["value"].GetBool());
          // TransparencyMode
          mi->SetTransparencyMode(
              (vzm::VzMI::TransparencyMode)parameters[1]["value"].GetInt());

          for (rapidjson::SizeType j = 2; j < parameters.Size(); ++j) {
            std::string name = parameters[j]["name"].GetString();
            vzm::UniformType type =
                (vzm::UniformType)parameters[j]["type"].GetInt();
            bool isSampler = parameters[j]["isSampler"].GetBool();

            const rapidjson::Value& values = parameters[j]["value"];
            if (values.IsArray()) {
              const rapidjson::Value& valueArr = values.GetArray();
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
               //TODO: 일단 임시로 _붙은 파라미터는 제외, 추후?
              if (name.find('_') == std::string::npos) {
                switch (type) {
                  case vzm::UniformType::BOOL:
                    if (isSampler) {
                      std::string relativePath = values.GetString();
                      if (relativePath.size() != 0) {
                        std::string absImagePath = res_path + relativePath;
                        vzm::VzTexture* texture =
                            (vzm::VzTexture*)vzm::NewResComponent(
                                vzm::RES_COMPONENT_TYPE::TEXTURE, "my image");
                        texture->ReadImage(absImagePath);
                        mi->SetTexture(name, texture->GetVID());
                      }
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
            }

            if (iter++ == pram.end()) {
              std::cerr << "pram error" << std::endl;
            }
          }
        }
      }
      break;
    }
    case vzm::SCENE_COMPONENT_TYPE::CAMERA: {
      vzm::VzCamera* camera = (vzm::VzCamera*)component;
      const rapidjson::Value& cameraNode = jsonNode["camera"];

      float near = cameraNode["near"].GetFloat();
      float far = cameraNode["far"].GetFloat();
      float fov = cameraNode["fov"].GetFloat();
      float ar = cameraNode["aspectratio"].GetFloat();
      camera->SetPerspectiveProjection(near, far, fov, ar);
      // cameraNode["focalLength"].GetFloat();
      float aperture = cameraNode["aperture"].GetFloat();
      float shutterSpeed = cameraNode["shutterSpeed"].GetFloat();
      float sensitivity = cameraNode["sensitivity"].GetFloat();
      camera->SetExposure(aperture, shutterSpeed, sensitivity);
      camera->SetFocusDistance(cameraNode["focusDistance"].GetFloat());
      break;
    }
    case vzm::SCENE_COMPONENT_TYPE::LIGHT: {
      vzm::VzLight* lightComponent = (vzm::VzLight*)component;
      const rapidjson::Value& lightNode = jsonNode["light"];

      lightComponent->SetType(
          (vzm::VzLight::Type)lightNode["lightType"].GetInt());
      lightComponent->SetIntensity(lightNode["intensity"].GetFloat());

      const rapidjson::Value& lightColor = lightNode["color"].GetArray();
      float lightColorArr[3] = {lightColor[0].GetFloat(),
                                lightColor[1].GetFloat(),
                                lightColor[2].GetFloat()};
      lightComponent->SetColor(lightColorArr);
      lightComponent->SetFalloff(lightNode["falloff"].GetFloat());
      lightComponent->SetSunHaloSize(lightNode["haloSize"].GetFloat());
      lightComponent->SetSunHaloFalloff(lightNode["haloFallOff"].GetFloat());
      lightComponent->SetSunAngularRadius(lightNode["sunRadius"].GetFloat());
      lightComponent->SetSpotLightCone(
          lightNode["spotLightInnerCone"].GetFloat(),
          lightNode["spotLightOuterCone"].GetFloat());

      lightComponent->SetShadowCaster(lightNode["shadowEnabled"].GetBool());
      vzm::VzLight::ShadowOptions sOpts = *lightComponent->GetShadowOptions();
      sOpts.mapSize = lightNode["mapSize"].GetInt();
      sOpts.stable = lightNode["stable"].GetBool();
      sOpts.lispsm = lightNode["lispsm"].GetBool();
      sOpts.shadowFar = lightNode["shadowFar"].GetFloat();

      const rapidjson::Value& lightDirection =
          lightNode["LightDirection"].GetArray();
      float lightDirectionArr[3] = {lightDirection[0].GetFloat(),
                                    lightDirection[1].GetFloat(),
                                    lightDirection[2].GetFloat()};
      lightComponent->SetDirection(lightDirectionArr);

      sOpts.vsm.elvsm = lightNode["elvsm"].GetBool();
      sOpts.vsm.blurWidth = lightNode["blurWidth"].GetFloat();
      sOpts.shadowCascades = lightNode["shadowCascades"].GetInt();
      sOpts.screenSpaceContactShadows =
          lightNode["screenSpaceContactShadows"].GetBool();
      const rapidjson::Value& splitPos = lightNode["splitPos"].GetArray();
      sOpts.cascadeSplitPositions[0] = splitPos[0].GetFloat();
      sOpts.cascadeSplitPositions[1] = splitPos[1].GetFloat();
      sOpts.cascadeSplitPositions[2] = splitPos[2].GetFloat();
      
      lightComponent->SetShadowOptions(sOpts);
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
        rapidjson::Value dsParamObj(rapidjson::kObjectType);
        dsParamObj.AddMember("name", rapidjson::Value("DoubleSided", allocator),
                             allocator);
        dsParamObj.AddMember("type", (int)vzm::UniformType::BOOL, allocator);
        dsParamObj.AddMember("isSampler", false, allocator);
        dsParamObj.AddMember("value", mi->IsDoubleSided(), allocator);
        parameters.PushBack(dsParamObj, allocator);

        // TransparencyMode
        rapidjson::Value tmParamObj(rapidjson::kObjectType);
        tmParamObj.AddMember(
            "name", rapidjson::Value("TransparencyMode", allocator), allocator);
        tmParamObj.AddMember("type", (int)vzm::UniformType::INT, allocator);
        tmParamObj.AddMember("isSampler", false, allocator);
        tmParamObj.AddMember("value", (int)mi->GetTransparencyMode(),
                             allocator);
        parameters.PushBack(tmParamObj, allocator);

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
            VID textureVID = mi->GetTexture(paramInfo.name);
            std::string path = "";
            if (textureVID != INVALID_VID) {
              vzm::VzTexture* texture =
                  (vzm::VzTexture*)vzm::GetVzComponent(textureVID);
              path = getRelativePath(texture->GetImageFileName());
            }
            paramObj.AddMember(
                "value", rapidjson::Value(path.c_str(), allocator), allocator);
          }

          parameters.PushBack(paramObj, allocator);
        }

        materialObj.AddMember("parameters", parameters, allocator);
        materials.PushBack(materialObj, allocator);
      }

      jsonNode.AddMember("materials", materials, allocator);
      break;
    }
    case vzm::SCENE_COMPONENT_TYPE::CAMERA: {
      vzm::VzCamera* camera = (vzm::VzCamera*)component;
      rapidjson::Value cameraSettings;
      cameraSettings.SetObject();

      float zNearP, zFarP, fovInDegree, aspectRatio;
      camera->GetPerspectiveProjection(&zNearP, &zFarP, &fovInDegree,
                                       &aspectRatio);

      cameraSettings.AddMember("near", zNearP, allocator);
      cameraSettings.AddMember("far", zFarP, allocator);
      cameraSettings.AddMember("fov", fovInDegree, allocator);
      cameraSettings.AddMember("aspectratio", aspectRatio, allocator);
      cameraSettings.AddMember("focalLength", camera->GetFocalLength(),
                               allocator);
      cameraSettings.AddMember("aperture", camera->GetAperture(), allocator);
      cameraSettings.AddMember("shutterSpeed", camera->GetShutterSpeed(),
                               allocator);
      cameraSettings.AddMember("sensitivity", camera->GetSensitivity(),
                               allocator);
      cameraSettings.AddMember("focusDistance", camera->GetFocusDistance(),
                               allocator);

      jsonNode.AddMember("camera", cameraSettings, allocator);
      break;
    }
    case vzm::SCENE_COMPONENT_TYPE::LIGHT: {
      vzm::VzLight* lightComponent = (vzm::VzLight*)component;
      rapidjson::Value lightSettings;
      lightSettings.SetObject();

      lightSettings.AddMember("lightType", (int)lightComponent->GetType(),
                              allocator);
      lightSettings.AddMember("intensity", lightComponent->GetIntensity(),
                              allocator);
      float color[3];
      lightComponent->GetColor(color);
      rapidjson::Value colorValue(rapidjson::kArrayType);
      colorValue.PushBack(color[0], allocator);
      colorValue.PushBack(color[1], allocator);
      colorValue.PushBack(color[2], allocator);
      lightSettings.AddMember("color", colorValue, allocator);

      lightSettings.AddMember("falloff", lightComponent->GetFalloff(),
                              allocator);

      lightSettings.AddMember("haloSize", lightComponent->GetSunHaloSize(),
                              allocator);
      lightSettings.AddMember("haloFallOff",
                              lightComponent->GetSunHaloFalloff(), allocator);
      lightSettings.AddMember("sunRadius",
                              lightComponent->GetSunAngularRadius(), allocator);
      lightSettings.AddMember("spotLightInnerCone",
                              lightComponent->GetSpotLightInnerCone(),
                              allocator);
      lightSettings.AddMember("spotLightOuterCone",
                              lightComponent->GetSpotLightOuterCone(),
                              allocator);
      // shadow
      vzm::VzLight::ShadowOptions sOpts = *lightComponent->GetShadowOptions();

      lightSettings.AddMember("shadowEnabled", lightComponent->IsShadowCaster(),
                              allocator);
      lightSettings.AddMember("mapSize", sOpts.mapSize, allocator);
      lightSettings.AddMember("stable", sOpts.stable, allocator);
      lightSettings.AddMember("lispsm", sOpts.lispsm, allocator);
      lightSettings.AddMember("shadowFar", sOpts.shadowFar, allocator);

      float fSunLightDirection[3];
      lightComponent->GetDirection(fSunLightDirection);
      rapidjson::Value lightDirectionValue(rapidjson::kArrayType);
      lightDirectionValue.PushBack(fSunLightDirection[0], allocator);
      lightDirectionValue.PushBack(fSunLightDirection[1], allocator);
      lightDirectionValue.PushBack(fSunLightDirection[2], allocator);
      lightSettings.AddMember("LightDirection", lightDirectionValue, allocator);

      lightSettings.AddMember("elvsm", sOpts.vsm.elvsm, allocator);
      lightSettings.AddMember("blurWidth", sOpts.vsm.blurWidth, allocator);

      lightSettings.AddMember("shadowCascades", sOpts.shadowCascades,
                              allocator);
      lightSettings.AddMember("screenSpaceContactShadows",
                              sOpts.screenSpaceContactShadows, allocator);
      rapidjson::Value splitPos(rapidjson::kArrayType);
      splitPos.PushBack(sOpts.cascadeSplitPositions[0], allocator);
      splitPos.PushBack(sOpts.cascadeSplitPositions[1], allocator);
      splitPos.PushBack(sOpts.cascadeSplitPositions[2], allocator);
      lightSettings.AddMember("splitPos", splitPos, allocator);

      jsonNode.AddMember("light", lightSettings, allocator);
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
    children.AddMember(rapidjson::Value(childName.c_str(), allocator),
                       childJsonNode, allocator);
  }
  jsonNode.AddMember("children", children, allocator);
}
void ImportGlobalSettings(const rapidjson::Value& globalSettings,
                          vzm::VzRenderer* g_renderer, vzm::VzScene* g_scene,
                          vzm::VzLight* g_light) {
  {
    const rapidjson::Value& view = globalSettings["view"];
    g_renderer->SetPostProcessingEnabled(
        view["IsPostProcessingEnabled"].GetBool());
    g_renderer->SetDitheringEnabled(view["IsDitheringEnabled"].GetBool());
    g_renderer->SetBloomEnabled(view["IsBloomEnabled"].GetBool());
    g_renderer->SetTaaEnabled(view["IsTaaEnabled"].GetBool());
    g_renderer->SetFxaaEnabled(view["IsFxaaEnabled"].GetBool());
    g_renderer->SetMsaaEnabled(view["IsMsaaEnabled"].GetBool());
    g_renderer->SetSsaoEnabled(view["IsSsaoEnabled"].GetBool());
    g_renderer->SetScreenSpaceReflectionsEnabled(
        view["IsScreenSpaceReflectionsEnabled"].GetBool());
    g_renderer->SetGuardBandEnabled(view["IsGuardBandEnabled"].GetBool());
  }
  {
    const rapidjson::Value& bloomOptions = globalSettings["bloomOptions"];
    g_renderer->SetBloomStrength(bloomOptions["BloomStrength"].GetFloat());
    g_renderer->SetBloomThreshold(bloomOptions["IsBloomThreshold"].GetBool());
    g_renderer->SetBloomLevels(bloomOptions["BloomLevels"].GetInt());
    g_renderer->SetBloomQuality(bloomOptions["BloomQuality"].GetInt());
    g_renderer->SetBloomLensFlare(bloomOptions["IsBloomLensFlare"].GetBool());
  }
  {
    const rapidjson::Value& taaOptions = globalSettings["taaOptions"];
    g_renderer->SetTaaUpscaling(taaOptions["IsTaaUpscaling"].GetBool());
    g_renderer->SetTaaHistoryReprojection(
        taaOptions["IsTaaHistoryReprojection"].GetBool());
    g_renderer->SetTaaFeedback(taaOptions["TaaFeedback"].GetFloat());
    g_renderer->SetTaaFilterHistory(taaOptions["IsTaaFilterHistory"].GetBool());
    g_renderer->SetTaaFilterInput(taaOptions["IsTaaFilterInput"].GetBool());
    g_renderer->SetTaaFilterWidth(taaOptions["TaaFilterWidth"].GetFloat());
    g_renderer->SetTaaLodBias(taaOptions["TaaLodBias"].GetFloat());
    g_renderer->SetTaaUseYCoCg(taaOptions["IsTaaUseYCoCg"].GetBool());
    g_renderer->SetTaaPreventFlickering(
        taaOptions["IsTaaPreventFlickering"].GetBool());
    g_renderer->SetTaaJitterPattern(
        (vzm::VzRenderer::JitterPattern)taaOptions["TaaJitterPattern"]
            .GetInt());
    g_renderer->SetTaaBoxClipping(
        (vzm::VzRenderer::BoxClipping)taaOptions["TaaBoxClipping"].GetInt());
    g_renderer->SetTaaBoxType(
        (vzm::VzRenderer::BoxType)taaOptions["TaaBoxType"].GetInt());
    g_renderer->SetTaaVarianceGamma(taaOptions["TaaVarianceGamma"].GetFloat());
    g_renderer->SetTaaSharpness(taaOptions["TaaSharpness"].GetFloat());
  }
  {
    const rapidjson::Value& ssaoOptions = globalSettings["ssaoOptions"];
    g_renderer->SetSsaoQuality(ssaoOptions["SsaoQuality"].GetInt());
    g_renderer->SetSsaoLowPassFilter(ssaoOptions["SsaoLowPassFilter"].GetInt());
    g_renderer->SetSsaoBentNormals(ssaoOptions["IsSsaoBentNormals"].GetBool());
    g_renderer->SetSsaoUpsampling(ssaoOptions["IsSsaoUpsampling"].GetBool());
    g_renderer->SetSsaoMinHorizonAngleRad(
        ssaoOptions["SsaoMinHorizonAngleRad"].GetFloat());
    g_renderer->SetSsaoBilateralThreshold(
        ssaoOptions["SsaoBilateralThreshold"].GetFloat());
    g_renderer->SetSsaoHalfResolution(
        ssaoOptions["IsSsaoHalfResolution"].GetBool());
    g_renderer->SetSsaoSsctEnabled(ssaoOptions["IsSsaoSsctEnabled"].GetBool());
    g_renderer->SetSsaoSsctLightConeRad(
        ssaoOptions["SsaoSsctLightConeRad"].GetFloat());
    g_renderer->SetSsaoSsctShadowDistance(
        ssaoOptions["SsaoSsctShadowDistance"].GetFloat());
    g_renderer->SetSsaoSsctContactDistanceMax(
        ssaoOptions["SsaoSsctContactDistanceMax"].GetFloat());
    g_renderer->SetSsaoSsctIntensity(
        ssaoOptions["SsaoSsctIntensity"].GetFloat());
    g_renderer->SetSsaoSsctDepthBias(
        ssaoOptions["SsaoSsctDepthBias"].GetFloat());
    g_renderer->SetSsaoSsctDepthSlopeBias(
        ssaoOptions["SsaoSsctDepthSlopeBias"].GetFloat());
    g_renderer->SetSsaoSsctSampleCount(
        ssaoOptions["SsaoSsctSampleCount"].GetInt());
    const rapidjson::Value& lightDirection =
        ssaoOptions["SsaoSsctLightDirection"].GetArray();
    float lightDirectionArr[3] = {lightDirection[0].GetFloat(),
                                  lightDirection[1].GetFloat(),
                                  lightDirection[2].GetFloat()};
    g_renderer->SetSsaoSsctLightDirection(lightDirectionArr);
  }
  {
    const rapidjson::Value& ssrOptions = globalSettings["ssrOptions"];
    g_renderer->SetScreenSpaceReflectionsThickness(
        ssrOptions["ScreenSpaceReflectionsThickness"].GetFloat());
    g_renderer->SetScreenSpaceReflectionsBias(
        ssrOptions["ScreenSpaceReflectionsBias"].GetFloat());
    g_renderer->SetScreenSpaceReflectionsMaxDistance(
        ssrOptions["ScreenSpaceReflectionsMaxDistance"].GetFloat());
    g_renderer->SetScreenSpaceReflectionsStride(
        ssrOptions["ScreenSpaceReflectionsStride"].GetFloat());
  }
  {
    const rapidjson::Value& dynamicResolution =
        globalSettings["dynamicResolution"];
    g_renderer->SetDynamicResoultionEnabled(
        dynamicResolution["IsDynamicResoultionEnabled"].GetBool());
    g_renderer->SetDynamicResoultionHomogeneousScaling(
        dynamicResolution["IsDynamicResoultionHomogeneousScaling"].GetBool());
    g_renderer->SetDynamicResoultionMinScale(
        dynamicResolution["DynamicResoultionMinScale"].GetFloat());
    g_renderer->SetDynamicResoultionMaxScale(
        dynamicResolution["DynamicResoultionMaxScale"].GetFloat());
    g_renderer->SetDynamicResoultionQuality(
        dynamicResolution["DynamicResoultionQuality"].GetInt());
    g_renderer->SetDynamicResoultionSharpness(
        dynamicResolution["DynamicResoultionSharpness"].GetFloat());
  }
  if(g_light){
    vzm::VzLight::ShadowOptions sOpts = *g_light->GetShadowOptions();

    const rapidjson::Value& LightSettings = globalSettings["LightSettings"];
    g_scene->SetIBLIntensity(LightSettings["IBLIntensity"].GetFloat());
    g_scene->SetIBLRotation(LightSettings["IBLRotation"].GetFloat());
    // TODO: sunlight enabled
    g_light->SetIntensity(LightSettings["Intensity"].GetFloat());
    g_light->SetSunHaloSize(LightSettings["SunHaloSize"].GetFloat());
    g_light->SetSunHaloFalloff(LightSettings["SunHaloFalloff"].GetFloat());
    g_light->SetSunAngularRadius(LightSettings["SunAngularRadius"].GetFloat());
    g_light->SetShadowCaster(LightSettings["shadowEnabled"].GetBool());

    sOpts.mapSize = LightSettings["mapSize"].GetInt();
    sOpts.stable = LightSettings["stable"].GetBool();
    sOpts.lispsm = LightSettings["lispsm"].GetBool();
    sOpts.shadowFar = LightSettings["shadowFar"].GetFloat();

    const rapidjson::Value& lightDirection =
        LightSettings["LightDirection"].GetArray();
    float lightDirectionArr[3] = {lightDirection[0].GetFloat(),
                                  lightDirection[1].GetFloat(),
                                  lightDirection[2].GetFloat()};
    g_light->SetDirection(lightDirectionArr);

    sOpts.vsm.elvsm = LightSettings["elvsm"].GetBool();
    sOpts.vsm.blurWidth = LightSettings["blurWidth"].GetFloat();
    sOpts.shadowCascades = LightSettings["shadowCascades"].GetInt();
    sOpts.screenSpaceContactShadows =
        LightSettings["screenSpaceContactShadows"].GetBool();
    const rapidjson::Value& splitPos = LightSettings["splitPos"].GetArray();
    sOpts.cascadeSplitPositions[0] = splitPos[0].GetFloat();
    sOpts.cascadeSplitPositions[1] = splitPos[1].GetFloat();
    sOpts.cascadeSplitPositions[2] = splitPos[2].GetFloat();

    g_renderer->SetShadowType(
        (vzm::VzRenderer::ShadowType)LightSettings["ShadowType"].GetInt());
    g_renderer->SetVsmHighPrecision(
        LightSettings["IsVsmHighPrecision"].GetBool());
    g_renderer->SetVsmMsaaSamples(LightSettings["VsmMsaaSamples"].GetInt());
    g_renderer->SetVsmAnisotropy(LightSettings["VsmAnisotropy"].GetInt());
    g_renderer->SetVsmMipmapping(LightSettings["IsVsmMipmapping"].GetBool());

    g_renderer->SetSoftShadowPenumbraScale(
        LightSettings["SoftShadowPenumbraScale"].GetFloat());
    g_renderer->SetSoftShadowPenumbraRatioScale(
        LightSettings["SoftShadowPenumbraRatioScale"].GetFloat());

    g_light->SetShadowOptions(sOpts);
  }
  {
    const rapidjson::Value& Fog = globalSettings["Fog"];
    g_renderer->SetFogEnabled(Fog["IsFogEnabled"].GetBool());
    g_renderer->SetFogDistance(Fog["FogDistance"].GetFloat());
    g_renderer->SetFogDensity(Fog["FogDensity"].GetFloat());
    g_renderer->SetFogHeight(Fog["FogHeight"].GetFloat());
    g_renderer->SetFogHeightFalloff(Fog["FogHeightFalloff"].GetFloat());
    g_renderer->SetFogInScatteringStart(Fog["FogInScatteringStart"].GetFloat());
    g_renderer->SetFogInScatteringSize(Fog["FogInScatteringSize"].GetFloat());
    g_renderer->SetFogExcludeSkybox(Fog["IsFogExcludeSkybox"].GetBool());
    g_renderer->SetFogColorSource(
        (vzm::VzRenderer::FogColorSource)Fog["FogColorSource"].GetInt());

    const rapidjson::Value& FogColor = Fog["FogColor"].GetArray();
    float fogColorArr[3] = {FogColor[0].GetFloat(), FogColor[1].GetFloat(),
                            FogColor[2].GetFloat()};
    g_renderer->SetFogColor(fogColorArr);
  }
  {
    const rapidjson::Value& Camera = globalSettings["Camera"];
    g_renderer->SetDofEnabled(Camera["IsDofEnabled"].GetBool());
    g_renderer->SetDofCocScale(Camera["DofCocScale"].GetFloat());
    g_renderer->SetDofCocAspectRatio(Camera["DofCocAspectRatio"].GetFloat());
    g_renderer->SetDofRingCount(Camera["DofRingCount"].GetInt());
    g_renderer->SetDofMaxCoc(Camera["DofMaxCoc"].GetInt());
    g_renderer->SetDofNativeResolution(
        Camera["IsDofNativeResolution"].GetBool());
    g_renderer->SetDofMedian(Camera["IsDofMedian"].GetBool());
    g_renderer->SetVignetteEnabled(Camera["IsVignetteEnabled"].GetBool());
    g_renderer->SetVignetteMidPoint(Camera["VignetteMidPoint"].GetFloat());
    g_renderer->SetVignetteRoundness(Camera["VignetteRoundness"].GetFloat());
    g_renderer->SetVignetteFeather(Camera["VignetteFeather"].GetFloat());
    const rapidjson::Value& VignetteColor = Camera["VignetteColor"].GetArray();
    float vignetteColorArr[3] = {VignetteColor[0].GetFloat(),
                                 VignetteColor[1].GetFloat(),
                                 VignetteColor[2].GetFloat()};
    g_renderer->SetFogColor(vignetteColorArr);
  }
  {
    const rapidjson::Value& Scene = globalSettings["Scene"];
    // TODO: Scene 추가
  }
}
void ExportGlobalSettings(rapidjson::Value& globalSettings,
                          rapidjson::Document::AllocatorType& allocator,
                          vzm::VzRenderer* g_renderer, vzm::VzScene* g_scene,
                          vzm::VzLight* g_light) {
  {
    rapidjson::Value view;
    view.SetObject();
    view.AddMember("IsPostProcessingEnabled",
                   g_renderer->IsPostProcessingEnabled(), allocator);
    view.AddMember("IsDitheringEnabled", g_renderer->IsDitheringEnabled(),
                   allocator);
    view.AddMember("IsBloomEnabled", g_renderer->IsBloomEnabled(), allocator);
    view.AddMember("IsTaaEnabled", g_renderer->IsTaaEnabled(), allocator);
    view.AddMember("IsFxaaEnabled", g_renderer->IsFxaaEnabled(), allocator);
    view.AddMember("IsMsaaEnabled", g_renderer->IsMsaaEnabled(), allocator);
    view.AddMember("IsSsaoEnabled", g_renderer->IsSsaoEnabled(), allocator);
    view.AddMember("IsScreenSpaceReflectionsEnabled",
                   g_renderer->IsScreenSpaceReflectionsEnabled(), allocator);
    view.AddMember("IsGuardBandEnabled", g_renderer->IsGuardBandEnabled(),
                   allocator);

    globalSettings.AddMember("view", view, allocator);
  }
  {
    rapidjson::Value bloomOptions;
    bloomOptions.SetObject();
    bloomOptions.AddMember("BloomStrength", g_renderer->GetBloomStrength(),
                           allocator);
    bloomOptions.AddMember("IsBloomThreshold", g_renderer->IsBloomThreshold(),
                           allocator);
    bloomOptions.AddMember("BloomLevels", g_renderer->GetBloomLevels(),
                           allocator);
    bloomOptions.AddMember("BloomQuality", g_renderer->GetBloomQuality(),
                           allocator);
    bloomOptions.AddMember("IsBloomLensFlare", g_renderer->IsBloomLensFlare(),
                           allocator);

    globalSettings.AddMember("bloomOptions", bloomOptions, allocator);
  }
  {
    rapidjson::Value taaOptions;
    taaOptions.SetObject();
    taaOptions.AddMember("IsTaaUpscaling", g_renderer->IsTaaUpscaling(),
                         allocator);
    taaOptions.AddMember("IsTaaHistoryReprojection",
                         g_renderer->IsTaaHistoryReprojection(), allocator);
    taaOptions.AddMember("TaaFeedback", g_renderer->GetTaaFeedback(),
                         allocator);
    taaOptions.AddMember("IsTaaFilterHistory", g_renderer->IsTaaFilterHistory(),
                         allocator);
    taaOptions.AddMember("IsTaaFilterInput", g_renderer->IsTaaFilterInput(),
                         allocator);
    taaOptions.AddMember("TaaFilterWidth", g_renderer->GetTaaFilterWidth(),
                         allocator);
    taaOptions.AddMember("TaaLodBias", g_renderer->GetTaaLodBias(), allocator);
    taaOptions.AddMember("IsTaaUseYCoCg", g_renderer->IsTaaUseYCoCg(),
                         allocator);
    taaOptions.AddMember("IsTaaPreventFlickering",
                         g_renderer->IsTaaPreventFlickering(), allocator);
    taaOptions.AddMember("TaaJitterPattern",
                         (int)g_renderer->GetTaaJitterPattern(), allocator);
    taaOptions.AddMember("TaaBoxClipping", (int)g_renderer->GetTaaBoxClipping(),
                         allocator);
    taaOptions.AddMember("TaaBoxType", (int)g_renderer->GetTaaBoxType(),
                         allocator);
    taaOptions.AddMember("TaaVarianceGamma", g_renderer->GetTaaVarianceGamma(),
                         allocator);
    taaOptions.AddMember("TaaSharpness", g_renderer->GetTaaSharpness(),
                         allocator);

    globalSettings.AddMember("taaOptions", taaOptions, allocator);
  }
  {
    rapidjson::Value ssaoOptions;
    ssaoOptions.SetObject();
    ssaoOptions.AddMember("SsaoQuality", g_renderer->GetSsaoQuality(),
                          allocator);
    ssaoOptions.AddMember("SsaoLowPassFilter",
                          g_renderer->GetSsaoLowPassFilter(), allocator);
    ssaoOptions.AddMember("IsSsaoBentNormals", g_renderer->IsSsaoBentNormals(),
                          allocator);
    ssaoOptions.AddMember("IsSsaoUpsampling", g_renderer->IsSsaoUpsampling(),
                          allocator);
    ssaoOptions.AddMember("SsaoMinHorizonAngleRad",
                          g_renderer->GetSsaoMinHorizonAngleRad(), allocator);
    ssaoOptions.AddMember("SsaoBilateralThreshold",
                          g_renderer->GetSsaoBilateralThreshold(), allocator);
    ssaoOptions.AddMember("IsSsaoHalfResolution",
                          g_renderer->IsSsaoHalfResolution(), allocator);
    // Dominant Light Shadows (experimental)
    ssaoOptions.AddMember("IsSsaoSsctEnabled", g_renderer->IsSsaoSsctEnabled(),
                          allocator);
    ssaoOptions.AddMember("SsaoSsctLightConeRad",
                          g_renderer->GetSsaoSsctLightConeRad(), allocator);
    ssaoOptions.AddMember("SsaoSsctShadowDistance",
                          g_renderer->GetSsaoSsctShadowDistance(), allocator);
    ssaoOptions.AddMember("SsaoSsctContactDistanceMax",
                          g_renderer->GetSsaoSsctContactDistanceMax(),
                          allocator);
    ssaoOptions.AddMember("SsaoSsctIntensity",
                          g_renderer->GetSsaoSsctIntensity(), allocator);
    ssaoOptions.AddMember("SsaoSsctDepthBias",
                          g_renderer->GetSsaoSsctDepthBias(), allocator);
    ssaoOptions.AddMember("SsaoSsctDepthSlopeBias",
                          g_renderer->GetSsaoSsctDepthSlopeBias(), allocator);
    ssaoOptions.AddMember("SsaoSsctSampleCount",
                          g_renderer->GetSsaoSsctSampleCount(), allocator);

    float lightDirection[3];
    g_renderer->GetSsaoSsctLightDirection(lightDirection);
    rapidjson::Value lightDirectionValue(rapidjson::kArrayType);
    lightDirectionValue.PushBack(lightDirection[0], allocator);
    lightDirectionValue.PushBack(lightDirection[1], allocator);
    lightDirectionValue.PushBack(lightDirection[2], allocator);
    ssaoOptions.AddMember("SsaoSsctLightDirection", lightDirectionValue,
                          allocator);

    globalSettings.AddMember("ssaoOptions", ssaoOptions, allocator);
  }
  {
    rapidjson::Value ssrOptions;
    ssrOptions.SetObject();
    ssrOptions.AddMember("ScreenSpaceReflectionsThickness",
                         g_renderer->GetScreenSpaceReflectionsThickness(),
                         allocator);
    ssrOptions.AddMember("ScreenSpaceReflectionsBias",
                         g_renderer->GetScreenSpaceReflectionsBias(),
                         allocator);
    ssrOptions.AddMember("ScreenSpaceReflectionsMaxDistance",
                         g_renderer->GetScreenSpaceReflectionsMaxDistance(),
                         allocator);
    ssrOptions.AddMember("ScreenSpaceReflectionsStride",
                         g_renderer->GetScreenSpaceReflectionsStride(),
                         allocator);

    globalSettings.AddMember("ssrOptions", ssrOptions, allocator);
  }
  {
    rapidjson::Value dynamicResolution;
    dynamicResolution.SetObject();
    dynamicResolution.AddMember("IsDynamicResoultionEnabled",
                                g_renderer->IsDynamicResoultionEnabled(),
                                allocator);
    dynamicResolution.AddMember(
        "IsDynamicResoultionHomogeneousScaling",
        g_renderer->IsDynamicResoultionHomogeneousScaling(), allocator);
    dynamicResolution.AddMember("DynamicResoultionMinScale",
                                g_renderer->GetDynamicResoultionMinScale(),
                                allocator);
    dynamicResolution.AddMember("DynamicResoultionMaxScale",
                                g_renderer->GetDynamicResoultionMaxScale(),
                                allocator);
    dynamicResolution.AddMember("DynamicResoultionQuality",
                                g_renderer->GetDynamicResoultionQuality(),
                                allocator);
    dynamicResolution.AddMember("DynamicResoultionSharpness",
                                g_renderer->GetDynamicResoultionSharpness(),
                                allocator);

    globalSettings.AddMember("dynamicResolution", dynamicResolution, allocator);
  }
  {
    rapidjson::Value LightSettings;
    LightSettings.SetObject();
    // TODO: ibl texture export -> ibl이 gltf 내부 경로에 있다는 보장이 필요함.
    LightSettings.AddMember("IBLIntensity", g_scene->GetIBLIntensity(),
                            allocator);
    LightSettings.AddMember("IBLRotation", g_scene->GetIBLRotation(),
                            allocator);
    // sunlight
    LightSettings.AddMember("Intensity", g_light->GetIntensity(), allocator);
    LightSettings.AddMember("SunHaloSize", g_light->GetSunHaloSize(),
                            allocator);
    LightSettings.AddMember("SunHaloFalloff", g_light->GetSunHaloFalloff(),
                            allocator);
    LightSettings.AddMember("SunAngularRadius", g_light->GetSunAngularRadius(),
                            allocator);

    // shadowsettings
    vzm::VzLight::ShadowOptions sOpts = *g_light->GetShadowOptions();
    LightSettings.AddMember("shadowEnabled", g_light->IsShadowCaster(),
                            allocator);
    LightSettings.AddMember("mapSize", sOpts.mapSize, allocator);
    LightSettings.AddMember("stable", sOpts.stable, allocator);
    LightSettings.AddMember("lispsm", sOpts.lispsm, allocator);
    LightSettings.AddMember("shadowFar", sOpts.shadowFar, allocator);

    float fSunLightDirection[3];
    g_light->GetDirection(fSunLightDirection);
    rapidjson::Value lightDirectionValue(rapidjson::kArrayType);
    lightDirectionValue.PushBack(fSunLightDirection[0], allocator);
    lightDirectionValue.PushBack(fSunLightDirection[1], allocator);
    lightDirectionValue.PushBack(fSunLightDirection[2], allocator);
    LightSettings.AddMember("LightDirection", lightDirectionValue, allocator);

    LightSettings.AddMember("elvsm", sOpts.vsm.elvsm, allocator);
    LightSettings.AddMember("blurWidth", sOpts.vsm.blurWidth, allocator);

    LightSettings.AddMember("shadowCascades", sOpts.shadowCascades, allocator);
    LightSettings.AddMember("screenSpaceContactShadows",
                            sOpts.screenSpaceContactShadows, allocator);
    rapidjson::Value splitPos(rapidjson::kArrayType);
    splitPos.PushBack(sOpts.cascadeSplitPositions[0], allocator);
    splitPos.PushBack(sOpts.cascadeSplitPositions[1], allocator);
    splitPos.PushBack(sOpts.cascadeSplitPositions[2], allocator);
    LightSettings.AddMember("splitPos", splitPos, allocator);
    // shadow settings
    LightSettings.AddMember("ShadowType", (int)g_renderer->GetShadowType(),
                            allocator);
    LightSettings.AddMember("IsVsmHighPrecision",
                            g_renderer->IsVsmHighPrecision(), allocator);
    LightSettings.AddMember("VsmMsaaSamples", g_renderer->GetVsmMsaaSamples(),
                            allocator);
    LightSettings.AddMember("VsmAnisotropy", g_renderer->GetVsmAnisotropy(),
                            allocator);
    LightSettings.AddMember("IsVsmMipmapping", g_renderer->IsVsmMipmapping(),
                            allocator);
    LightSettings.AddMember("SoftShadowPenumbraScale",
                            g_renderer->GetSoftShadowPenumbraScale(),
                            allocator);
    LightSettings.AddMember("SoftShadowPenumbraRatioScale",
                            g_renderer->GetSoftShadowPenumbraRatioScale(),
                            allocator);

    globalSettings.AddMember("LightSettings", LightSettings, allocator);
  }
  {
    rapidjson::Value Fog;
    Fog.SetObject();
    Fog.AddMember("IsFogEnabled", g_renderer->IsFogEnabled(), allocator);
    Fog.AddMember("FogDistance", g_renderer->GetFogDistance(), allocator);
    Fog.AddMember("FogDensity", g_renderer->GetFogDensity(), allocator);
    Fog.AddMember("FogHeight", g_renderer->GetFogHeight(), allocator);
    Fog.AddMember("FogHeightFalloff", g_renderer->GetFogHeightFalloff(),
                  allocator);
    Fog.AddMember("FogInScatteringStart", g_renderer->GetFogInScatteringStart(),
                  allocator);
    Fog.AddMember("FogInScatteringSize", g_renderer->GetFogInScatteringSize(),
                  allocator);
    Fog.AddMember("IsFogExcludeSkybox", g_renderer->IsFogExcludeSkybox(),
                  allocator);
    Fog.AddMember("FogColorSource", (int)g_renderer->GetFogColorSource(),
                  allocator);
    float fogColor[3];
    g_renderer->GetFogColor(fogColor);
    rapidjson::Value fogColorValue(rapidjson::kArrayType);
    fogColorValue.PushBack(fogColor[0], allocator);
    fogColorValue.PushBack(fogColor[1], allocator);
    fogColorValue.PushBack(fogColor[2], allocator);
    Fog.AddMember("FogColor", fogColorValue, allocator);

    globalSettings.AddMember("Fog", Fog, allocator);
  }
  {
    rapidjson::Value Camera;
    Camera.SetObject();
    // Dof
    Camera.AddMember("IsDofEnabled", g_renderer->IsDofEnabled(), allocator);
    Camera.AddMember("DofCocScale", g_renderer->GetDofCocScale(), allocator);
    Camera.AddMember("DofCocAspectRatio", g_renderer->GetDofCocAspectRatio(),
                     allocator);
    Camera.AddMember("DofRingCount", g_renderer->GetDofRingCount(), allocator);
    Camera.AddMember("DofMaxCoc", g_renderer->GetDofMaxCoc(), allocator);
    Camera.AddMember("IsDofNativeResolution",
                     g_renderer->IsDofNativeResolution(), allocator);
    Camera.AddMember("IsDofMedian", g_renderer->IsDofMedian(), allocator);
    // Vignette
    Camera.AddMember("IsVignetteEnabled", g_renderer->IsVignetteEnabled(),
                     allocator);
    Camera.AddMember("VignetteMidPoint", g_renderer->GetVignetteMidPoint(),
                     allocator);
    Camera.AddMember("VignetteRoundness", g_renderer->GetVignetteRoundness(),
                     allocator);
    Camera.AddMember("VignetteFeather", g_renderer->GetVignetteFeather(),
                     allocator);
    float vignetteColor[3];
    g_renderer->GetVignetteColor(vignetteColor);
    rapidjson::Value vignetteColorValue(rapidjson::kArrayType);
    vignetteColorValue.PushBack(vignetteColor[0], allocator);
    vignetteColorValue.PushBack(vignetteColor[1], allocator);
    vignetteColorValue.PushBack(vignetteColor[2], allocator);
    Camera.AddMember("VignetteColor", vignetteColorValue, allocator);

    globalSettings.AddMember("Camera", Camera, allocator);
  }
  {
    rapidjson::Value Scene;
    Scene.SetObject();
    globalSettings.AddMember("Scene", Scene, allocator);
  }
}
void importSettings(VID root, std::string filePath, vzm::VzRenderer* renderer,
                    vzm::VzScene* scene, vzm::VzLight* sunLight) {
  FILE* fp;
  fopen_s(&fp, filePath.c_str(), "r");
  if (fp == nullptr) return;

  char readBuffer[65536];
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  rapidjson::Document document;
  document.ParseStream(is);
  fclose(fp);

  vzm::VzSceneComp* component = (vzm::VzSceneComp*)vzm::GetVzComponent(root);

  ImportMaterials(document["hierarchy"], component);

  ImportGlobalSettings(document["global"], renderer, scene, sunLight);
}

void exportSettings(VID root, vzm::VzRenderer* renderer, vzm::VzScene* scene,
                    vzm::VzLight* sunLight) {
  FILE* outfp;
  rapidjson::Document outputDoc;

  outputDoc.SetObject();

  rapidjson::Value hierarchy;
  ExportMaterials(hierarchy, outputDoc.GetAllocator(), root);
  outputDoc.AddMember("hierarchy", hierarchy, outputDoc.GetAllocator());

  rapidjson::Value globalSettings;
  globalSettings.SetObject();
  ExportGlobalSettings(globalSettings, outputDoc.GetAllocator(), renderer,
                       scene, sunLight);
  outputDoc.AddMember("global", globalSettings, outputDoc.GetAllocator());

  time_t rawtime;
  time(&rawtime);
  struct tm local_timeinfo;
  localtime_s(&local_timeinfo, &rawtime);

  std::string exportFileName =
      std::format("savefile[{:02}-{:02}-{:02}].json", local_timeinfo.tm_hour,
                  local_timeinfo.tm_min, local_timeinfo.tm_sec);
  std::cout << exportFileName << std::endl;
  fopen_s(&outfp, exportFileName.c_str(), "w");
  if (outfp == nullptr) return;

  char writeBuffer[65536];
  rapidjson::FileWriteStream os(outfp, writeBuffer, sizeof(writeBuffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
  outputDoc.Accept(writer);
  fclose(outfp);
}

}  // namespace savefileIO
