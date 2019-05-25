// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "upsample.h"
#include "weights_reorder.h"
#include "network.h"

namespace oidn {

  template<int K>
  Network<K>::Network(const std::map<std::string, Tensor>& weightMap)
    : eng(engine::cpu, 0),
      sm(eng),
      weightMap(weightMap)
  {
  }

  template<int K>
  void Network<K>::execute(ProgressMonitorFunction progressFunc, void* progressUserPtr)
  {
    if (progressFunc && !progressFunc(progressUserPtr, 0))
      throw Exception(Error::Cancelled, "execution was cancelled");

    for (size_t i = 0; i < nodes.size(); ++i)
    {
      nodes[i]->execute(sm);

      if (progressFunc && !progressFunc(progressUserPtr, double(i+1) / double(nodes.size())))
        throw Exception(Error::Cancelled, "execution was cancelled");
    }
  }

  template<int K>
  std::shared_ptr<memory> Network<K>::allocTensor(const memory::dims& dims,
                                                  memory::format_tag format,
                                                  void* data)
  {
    if (format == memory::format_tag::any)
    {
      if (dims.size() == 4)
        format = BlockedFormat<K>::nChwKc;
      else if (dims.size() == 1)
        format = memory::format_tag::x;
      else
        assert(0);
    }
    memory::desc desc(dims, memory::data_type::f32, format);
    if (data == nullptr)
      return std::make_shared<memory>(desc, eng);
    else;
      return std::make_shared<memory>(desc, eng, data);
  }

  template<int K>
  std::shared_ptr<memory> Network<K>::castTensor(const memory::dims& dims,
                                                 const std::shared_ptr<memory>& src,
                                                 size_t srcOffset,
                                                 memory::format_tag format)
  {
    const mkldnn_memory_desc_t& srcDesc = src->get_desc().data;
    MAYBE_UNUSED(srcDesc);
    assert(srcDesc.data_type == memory::data_type::f32);
    assert(getTensorSize(src) >= srcOffset + getTensorSize(dims));

    if (format == memory::format_tag::any)
    {
      if (dims.size() == 4)
        format = BlockedFormat<K>::nChwKc;
      else if (dims.size() == 1)
        format = memory::format_tag::x;
      else
        assert(0);
    }
    memory::desc desc(dims, memory::data_type::f32, format);
    float* srcPtr = (float*)src->get_data_handle() + srcOffset;
    return std::make_shared<memory>(desc, eng, srcPtr);
  }

  template<int K>
  std::shared_ptr<memory> Network<K>::castTensor(const memory::dims& dims,
                                                 const std::shared_ptr<memory>& src,
                                                 const memory::dims& srcOffset)
  {
    return castTensor(dims, src, getTensorSize(srcOffset));
  }

  template<int K>
  void Network<K>::zeroTensor(const std::shared_ptr<memory>& dst)
  {
    assert(getTensorType(dst) == memory::data_type::f32);
    memset(dst->get_data_handle(), 0, getTensorSize(dst)*sizeof(float));
  }

  template<int K>
  memory::dims Network<K>::getInputReorderDims(const memory::dims& srcDims, int spatialPad)
  {
    memory::dims dstDims = srcDims;
    dstDims[1] = getPadded<K>(srcDims[1]); // round up C
    dstDims[2] = (srcDims[2] + spatialPad - 1) / spatialPad * spatialPad; // round up H
    dstDims[3] = (srcDims[3] + spatialPad - 1) / spatialPad * spatialPad; // round up W
    return dstDims;
  }

  template<int K>
  memory::dims Network<K>::getConvDims(const std::string& name, const memory::dims& srcDims)
  {
    auto b = weightMap[name + "/b"];
    memory::dims dstDims = srcDims;
    dstDims[1] = getPadded<K>(b.dims[0]); // dstDims[C] = getPadded(OC)
    return dstDims;
  }

  template<int K>
  std::shared_ptr<Node> Network<K>::addConv(const std::string& name,
                                            const std::shared_ptr<memory>& src,
                                            const std::shared_ptr<memory>& userDst,
                                            bool relu)
  {
    const memory::dims strides = {1, 1};
    const memory::dims padding = {1, 1};

    memory::dims srcDims = getTensorDims(src);

    // Get the weights
    const auto& W = weightMap[name + "/W"];
    if (W.ndims() != 4 || W.format != "oihw")
      throw Exception(Error::InvalidOperation, "invalid convolution weights");
    memory::dims weightsDims = W.dims;
    auto userWeights = allocTensor(weightsDims, memory::format_tag::oihw, W.data);

    // Pad the weights
    memory::dims weightsPadDims = weightsDims;
    weightsPadDims[1] = getPadded<K>(weightsDims[1]); // IC
    weightsPadDims[0] = getPadded<K>(weightsDims[0]); // OC
    assert(srcDims[1] == weightsPadDims[1]); // srcDims[C] == weightsPadDims[IC]
    auto weightsPad = allocTensor(weightsPadDims, memory::format_tag::oihw);
    WeightsReorderNode<K>(userWeights, weightsPad).execute(sm);

    // Get the biases
    const auto& b = weightMap[name + "/b"];
    if (b.ndims() != 1)
      throw Exception(Error::InvalidOperation, "invalid convolution biases");
    memory::dims biasDims = b.dims;

    // Copy/pad the biases
    memory::dims biasPadDims = {getPadded<K>(biasDims[0])};
    auto bias = allocTensor(biasPadDims);
    if (biasDims[0] != biasPadDims[0])
      memset(bias->get_data_handle(), 0, biasPadDims[0]*sizeof(float));
    memcpy(bias->get_data_handle(), b.data, biasDims[0]*sizeof(float));

    // Allocate memory for destination
    memory::dims dstDims = srcDims;
    dstDims[1] = weightsPadDims[0]; // dstDims[C] = weightsPadDims[OC]

    std::shared_ptr<memory> dst;
    if (!userDst)
      dst = allocTensor(dstDims);
    else if (getTensorDims(userDst) == dstDims)
      dst = userDst;
    else
      dst = castTensor(dstDims, userDst);

    // Create a convolution
    // Let the convolution primitive choose the weights format
    auto weightsDesc = memory::desc({ weightsPadDims }, memory::data_type::f32, memory::format_tag::any);

    auto convAlgo = (K == 16) ? convolution_winograd : convolution_direct;
    auto convDesc = convolution_forward::desc(
      prop_kind::forward_inference, convAlgo,
      src->get_desc(),
      weightsDesc,
      bias->get_desc(),
      dst->get_desc(),
      strides, padding, padding, padding_kind::zero);

    // Incorporate relu
    mkldnn::primitive_attr convAttr;
    if (relu)
    {
      mkldnn::post_ops ops;
      ops.append_eltwise(
        1.f,   // scale factor, not used
        algorithm::eltwise_relu,
        0.f,   // max with
        0.f    // unused
      );
      convAttr.set_post_ops(ops);
    }
    convAttr.set_scratchpad_mode(scratchpad_mode_user);

    auto convPrimDesc = convolution_forward::primitive_desc(convDesc, convAttr, eng);

    // Reorder the weights to the final format, if necessary
    auto weights = weightsPad;
    if (convPrimDesc.weights_desc() != weightsPad->get_desc())
    {
      weights = std::make_shared<memory>(convPrimDesc.weights_desc(), eng);
      ReorderNode(weightsPad, weights).execute(sm);
    }

    // Create convolution node and add it to the net
    auto node = std::make_shared<ConvNode>(convPrimDesc, src, weights, bias, dst);
    nodes.push_back(node);
    return node;
  }

  template<int K>
  memory::dims Network<K>::getPoolDims(const memory::dims& srcDims)
  {
    memory::dims dstDims = srcDims;
    dstDims[2] /= 2; // H/2
    dstDims[3] /= 2; // W/2
    return dstDims;
  }

  template<int K>
  std::shared_ptr<Node> Network<K>::addPool(const std::shared_ptr<memory>& src,
                                            const std::shared_ptr<memory>& userDst)
  {
    const memory::dims kernel  = {2, 2};
    const memory::dims strides = {2, 2};
    const memory::dims padding = {0, 0};

    memory::dims srcDims = getTensorDims(src);
    memory::dims dstDims = getPoolDims(srcDims);

    std::shared_ptr<memory> dst;
    if (!userDst)
      dst = allocTensor(dstDims);
    else if (getTensorDims(userDst) == dstDims)
      dst = userDst;
    else
      dst = castTensor(dstDims, userDst);

    auto poolDesc = pooling_forward::desc(
      prop_kind::forward_inference, pooling_max,
      src->get_desc(),
      dst->get_desc(),
      strides, kernel, padding, padding, padding_kind::zero);

    mkldnn::primitive_attr poolAttr;
    poolAttr.set_scratchpad_mode(scratchpad_mode_user);

    auto poolPrimDesc = pooling_forward::primitive_desc(poolDesc, poolAttr, eng);

    auto node = std::make_shared<PoolNode>(poolPrimDesc, src, dst);
    nodes.push_back(node);
    return node;
  }

  template<int K>
  memory::dims Network<K>::getUpsampleDims(const memory::dims& srcDims)
  {
    memory::dims dstDims = srcDims;
    dstDims[2] *= 2; // H*2
    dstDims[3] *= 2; // W*2
    return dstDims;
  }

  template<int K>
  std::shared_ptr<Node> Network<K>::addUpsample(const std::shared_ptr<memory>& src,
                                                const std::shared_ptr<memory>& userDst)
  {
    memory::dims srcDims = getTensorDims(src);
    memory::dims dstDims = getUpsampleDims(srcDims);

    std::shared_ptr<memory> dst;
    if (!userDst)
      dst = allocTensor(dstDims);
    else if (getTensorDims(userDst) == dstDims)
      dst = userDst;
    else
      dst = castTensor(dstDims, userDst);

    // Create upsampling node and add it to net
    auto node = std::make_shared<UpsampleNode<K>>(src, dst);
    nodes.push_back(node);
    return node;
  }

  template<int K>
  memory::dims Network<K>::getConcatDims(const memory::dims& src1Dims, const memory::dims& src2Dims)
  {
    assert(src1Dims[0] == src2Dims[0]); // N
    assert(src1Dims[2] == src2Dims[2]); // H
    assert(src1Dims[3] == src2Dims[3]); // W

    memory::dims dstDims = src1Dims;
    dstDims[1] += src2Dims[1]; // C
    return dstDims;
  }

  template<int K>
  std::shared_ptr<Node> Network<K>::addAutoexposure(const Image& color,
                                                    const std::shared_ptr<HDRTransferFunction>& transferFunc)
  {
    auto node = std::make_shared<AutoexposureNode>(color, transferFunc);
    nodes.push_back(node);
    return node;
  }

  template <int K>
  void Network<K>::finalize()
  {
    // Compute the size of the scratchpad
    size_t scratchpadSize = 0;
    for (const auto& node : nodes)
      scratchpadSize = max(scratchpadSize, node->getScratchpadSize());

    // Allocate the scratchpad
    memory::dims scratchpadDims = { memory::dim(scratchpadSize) };
    memory::desc scratchpadDesc(scratchpadDims, memory::data_type::u8, memory::format_tag::x);
    auto scratchpad = std::make_shared<memory>(scratchpadDesc, eng);

    // Set the scratchpad for the nodes
    for (auto& node : nodes)
      node->setScratchpad(scratchpad);
  }

  template class Network<8>;
  template class Network<16>;

} // namespace oidn
