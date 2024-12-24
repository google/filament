/// @brief Helper functions to create generic texture
/// @file gli/make_texture.hpp

#pragma once

namespace gli
{
	// Helper function to create a 1d texture with a specific number of levels
	gli::texture make_texture1d(format Format, extent1d const& Extent, size_t Levels);

	// Helper function to create a 1d texture with a complete mipmap chain
	gli::texture make_texture1d(format Format, extent1d const& Extent);

	// Helper function to create a 1d array texture with a specific number of levels
	gli::texture make_texture1d_array(format Format, extent1d const& Extent, size_t Layers, size_t Levels);

	// Helper function to create a 1d array texture with a complete mipmap chain
	gli::texture make_texture1d_array(format Format, extent1d const& Extent, size_t Layers);

	// Helper function to create a 2d texture with a specific number of levels
	gli::texture make_texture2d(format Format, extent2d const& Extent, size_t Levels);

	// Helper function to create a 2d texture with a complete mipmap chain
	gli::texture make_texture2d(format Format, extent2d const& Extent);

	// Helper function to create a 2d array texture with a specific number of levels
	gli::texture make_texture2d_array(format Format, extent2d const& Extent, size_t Layer, size_t Levels);

	// Helper function to create a 2d array texture with a complete mipmap chain
	gli::texture make_texture2d_array(format Format, extent2d const& Extent, size_t Layer);

	// Helper function to create a 3d texture with a specific number of levels
	gli::texture make_texture3d(format Format, extent3d const& Extent, size_t Levels);

	// Helper function to create a 3d texture with a complete mipmap chain
	gli::texture make_texture3d(format Format, extent3d const& Extent);

	// Helper function to create a cube texture with a specific number of levels
	gli::texture make_texture_cube(format Format, extent2d const& Extent, size_t Levels);

	// Helper function to create a cube texture with a complete mipmap chain
	gli::texture make_texture_cube(format Format, extent2d const& Extent);

	// Helper function to create a cube array texture with a specific number of levels
	gli::texture make_texture_cube_array(format Format, extent2d const& Extent, size_t Layer, size_t Levels);

	// Helper function to create a cube array texture with a complete mipmap chain
	gli::texture make_texture_cube_array(format Format, extent2d const& Extent, size_t Layer);
}//namespace gli

#include "./core/make_texture.inl"
