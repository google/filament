// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

namespace dawn::native {

wgpu::FeatureName ToAPI(Feature feature) {
  switch (feature) {
    {% for enum in types["feature name"].values if (enum.valid and not is_enum_value_proxy(enum)) %}
      case Feature::{{as_cppEnum(enum.name)}}:
        return wgpu::FeatureName::{{as_cppEnum(enum.name)}};
    {% endfor %}
    case Feature::InvalidEnum:
      break;
  }
  DAWN_UNREACHABLE();
}

Feature FromAPI(wgpu::FeatureName feature) {
  switch (feature) {
    {% for enum in types["feature name"].values if not is_enum_value_proxy(enum) %}
      case wgpu::FeatureName::{{as_cppEnum(enum.name)}}:
        {% if enum.valid %}
          return Feature::{{as_cppEnum(enum.name)}};
        {% else %}
          return Feature::InvalidEnum;
        {% endif %}
    {% endfor %}
    default:
      return Feature::InvalidEnum;
  }
}

static constexpr bool FeatureInfoIsDefined(Feature feature) {
  for (const auto& info : kFeatureInfo) {
    if (info.feature == feature) {
      return true;
    }
  }
  return false;
}

static constexpr ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> InitializeFeatureEnumAndInfoList() {
  constexpr size_t kInfoCount = sizeof(kFeatureInfo) / sizeof(kFeatureInfo[0]);
  ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> list{};
  {% for enum in types["feature name"].values if (enum.valid and not is_enum_value_proxy(enum)) %}
    {
      static_assert(FeatureInfoIsDefined(Feature::{{as_cppEnum(enum.name)}}),
                    "Please define feature info for {{as_cppEnum(enum.name)}} in Features.cpp");
      for (size_t i = 0; i < kInfoCount; ++i) {
        if (kFeatureInfo[i].feature == Feature::{{as_cppEnum(enum.name)}}) {
          list[Feature::{{as_cppEnum(enum.name)}}] = {
            //* Match feature name casing with javascript casing.
            "{{enum.name.js_enum_case()}}",
            kFeatureInfo[i].info.description,
            kFeatureInfo[i].info.url,
            kFeatureInfo[i].info.featureState,
          };
        }
      }
    }
  {% endfor %}
  return list;
}

const ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> kFeatureNameAndInfoList = InitializeFeatureEnumAndInfoList();

}
