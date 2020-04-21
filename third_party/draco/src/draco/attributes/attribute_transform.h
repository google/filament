// Copyright 2017 The Draco Authors.
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
#ifndef DRACO_ATTRIBUTES_ATTRIBUTE_TRANSFORM_H_
#define DRACO_ATTRIBUTES_ATTRIBUTE_TRANSFORM_H_

#include "draco/attributes/attribute_transform_data.h"
#include "draco/attributes/point_attribute.h"

namespace draco {

// Virtual base class for various attribute transforms, enforcing common
// interface where possible.
class AttributeTransform {
 public:
  virtual ~AttributeTransform() = default;

  // Return attribute transform type.
  virtual AttributeTransformType Type() const = 0;
  // Try to init transform from attribute.
  virtual bool InitFromAttribute(const PointAttribute &attribute) = 0;
  // Copy parameter values into the provided AttributeTransformData instance.
  virtual void CopyToAttributeTransformData(
      AttributeTransformData *out_data) const = 0;
  bool TransferToAttribute(PointAttribute *attribute) const;

 protected:
  std::unique_ptr<PointAttribute> InitPortableAttribute(
      int num_entries, int num_components, int num_points,
      const PointAttribute &attribute, bool is_unsigned) const;
};

}  // namespace draco

#endif  // DRACO_ATTRIBUTES_ATTRIBUTE_OCTAHEDRON_TRANSFORM_H_
