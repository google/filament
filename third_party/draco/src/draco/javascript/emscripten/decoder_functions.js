// Copyright 2020 The Draco Authors.
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

// Returns encoded geometry type stored in the |array|. In general, |array|
// should be a javascript Int8Array containing the encoded data. For backward
// compatibility, |array| can also represent a Module.DecoderBuffer object.
Module['Decoder'].prototype.GetEncodedGeometryType = function(array) {
  if (array.__class__ && array.__class__ === Module.DecoderBuffer) {
    // |array| is a DecoderBuffer. Pass it to the deprecated function.
    return Module.Decoder.prototype.GetEncodedGeometryType_Deprecated(array);
  }
  if (array.byteLength < 8)
    return Module.INVALID_GEOMETRY_TYPE;
  switch (array[7]) {
    case 0:
      return Module.POINT_CLOUD;
    case 1:
      return Module.TRIANGULAR_MESH;
    default:
      return Module.INVALID_GEOMETRY_TYPE;
  }
};
