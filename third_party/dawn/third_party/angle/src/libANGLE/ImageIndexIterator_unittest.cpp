//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/utilities.h"
#include "libANGLE/ImageIndex.h"

using namespace gl;

namespace
{

static const GLint minMip   = 0;
static const GLint maxMip   = 4;
static const GLint minLayer = 1;
static const GLint maxLayer = 3;

TEST(ImageIndexTest, Iterator2D)
{
    ImageIndexIterator iter = ImageIndexIterator::Make2D(minMip, maxMip);

    ASSERT_GE(0, minMip);

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        EXPECT_TRUE(iter.hasNext());
        ImageIndex current   = iter.current();
        ImageIndex nextIndex = iter.next();

        EXPECT_EQ(TextureType::_2D, nextIndex.getType());
        EXPECT_EQ(TextureTarget::_2D, nextIndex.getTarget());
        EXPECT_EQ(mip, nextIndex.getLevelIndex());
        EXPECT_FALSE(nextIndex.hasLayer());
        EXPECT_FALSE(nextIndex.has3DLayer());

        // Also test current
        EXPECT_EQ(current.getType(), nextIndex.getType());
        EXPECT_EQ(current.getLevelIndex(), nextIndex.getLevelIndex());
        EXPECT_EQ(current.getLayerIndex(), nextIndex.getLayerIndex());
    }

    EXPECT_FALSE(iter.hasNext());
}

TEST(ImageIndexTest, IteratorCube)
{
    ImageIndexIterator iter = ImageIndexIterator::MakeCube(minMip, maxMip);

    ASSERT_GE(0, minMip);

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        for (TextureTarget target : AllCubeFaceTextureTargets())
        {
            EXPECT_TRUE(iter.hasNext());
            ImageIndex nextIndex = iter.next();

            EXPECT_EQ(TextureType::CubeMap, nextIndex.getType());
            EXPECT_EQ(target, nextIndex.getTarget());
            EXPECT_EQ(mip, nextIndex.getLevelIndex());
            EXPECT_TRUE(nextIndex.hasLayer());
            EXPECT_FALSE(nextIndex.has3DLayer());
        }
    }

    EXPECT_FALSE(iter.hasNext());
}

TEST(ImageIndexTest, Iterator3D)
{
    ImageIndexIterator iter = ImageIndexIterator::Make3D(minMip, maxMip, minLayer, maxLayer);

    ASSERT_GE(0, minMip);

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        for (GLint layer = minLayer; layer < maxLayer; layer++)
        {
            EXPECT_TRUE(iter.hasNext());
            ImageIndex nextIndex = iter.next();

            EXPECT_EQ(TextureType::_3D, nextIndex.getType());
            EXPECT_EQ(TextureTarget::_3D, nextIndex.getTarget());
            EXPECT_EQ(mip, nextIndex.getLevelIndex());
            EXPECT_EQ(layer, nextIndex.getLayerIndex());
            EXPECT_TRUE(nextIndex.hasLayer());
            EXPECT_TRUE(nextIndex.has3DLayer());
        }
    }

    EXPECT_FALSE(iter.hasNext());
}

TEST(ImageIndexTest, Iterator2DArray)
{
    GLsizei layerCounts[] = {1, 3, 5, 2};

    ImageIndexIterator iter = ImageIndexIterator::Make2DArray(minMip, maxMip, layerCounts);

    ASSERT_GE(0, minMip);
    ASSERT_EQ(ArraySize(layerCounts), static_cast<size_t>(maxMip));

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        for (GLint layer = 0; layer < layerCounts[mip]; layer++)
        {
            EXPECT_TRUE(iter.hasNext());
            ImageIndex nextIndex = iter.next();

            EXPECT_EQ(TextureType::_2DArray, nextIndex.getType());
            EXPECT_EQ(TextureTarget::_2DArray, nextIndex.getTarget());
            EXPECT_EQ(mip, nextIndex.getLevelIndex());
            EXPECT_EQ(layer, nextIndex.getLayerIndex());
            EXPECT_TRUE(nextIndex.hasLayer());
            EXPECT_TRUE(nextIndex.has3DLayer());
        }
    }

    EXPECT_FALSE(iter.hasNext());
}

TEST(ImageIndexTest, LayerIterator2DArray)
{
    GLsizei layerCounts[] = {1, 3, 5, 2};

    ASSERT_GE(0, minMip);
    ASSERT_EQ(ArraySize(layerCounts), static_cast<size_t>(maxMip));

    for (GLint mip = minMip; mip < maxMip; mip++)
    {
        // Make a layer iterator.
        ImageIndex mipImageIndex = ImageIndex::Make2DArray(mip, ImageIndex::kEntireLevel);
        ImageIndexIterator iter  = mipImageIndex.getLayerIterator(layerCounts[mip]);

        for (GLint layer = 0; layer < layerCounts[mip]; layer++)
        {
            EXPECT_TRUE(iter.hasNext());
            ImageIndex nextIndex = iter.next();

            EXPECT_EQ(TextureType::_2DArray, nextIndex.getType());
            EXPECT_EQ(TextureTarget::_2DArray, nextIndex.getTarget());
            EXPECT_EQ(mip, nextIndex.getLevelIndex());
            EXPECT_EQ(layer, nextIndex.getLayerIndex());
            EXPECT_TRUE(nextIndex.hasLayer());
            EXPECT_TRUE(nextIndex.has3DLayer());
            EXPECT_LT(nextIndex.getLayerIndex(), layerCounts[mip]);
        }

        EXPECT_FALSE(iter.hasNext());
    }
}

}  // namespace
