/// @brief Include to use images, a representation of a single texture level.
/// @file gli/image.hpp

#pragma once

#include "./core/storage_linear.hpp"

namespace gli
{
	/// Image, representation for a single texture level
	class image
	{
	private:
		friend class texture1d;
		friend class texture2d;
		friend class texture3d;

	public:
		typedef size_t size_type;
		typedef gli::format format_type;
		typedef storage_linear::extent_type extent_type;
		typedef storage_linear::data_type data_type;

		/// Create an empty image instance
		image();

		/// Create an image object and allocate an image storoge for it.
		explicit image(format_type Format, extent_type const& Extent);

		/// Create an image object by sharing an existing image storage_linear from another image instance.
		/// This image object is effectively an image view where format can be reinterpreted
		/// with a different compatible image format.
		/// For formats to be compatible, the block size of source and destination must match.
		explicit image(image const& Image, format_type Format);

		/// Return whether the image instance is empty, no storage_linear or description have been assigned to the instance.
		bool empty() const;

		/// Return the image instance format.
		format_type format() const;

		/// Return the dimensions of an image instance: width, height and depth.
		extent_type extent() const;

		/// Return the memory size of an image instance storage_linear in bytes.
		size_type size() const;

		/// Return the number of blocks contained in an image instance storage_linear.
		/// genType size must match the block size conresponding to the image format. 
		template <typename genType>
		size_type size() const;

		/// Return a pointer to the beginning of the image instance data.
		void* data();

		/// Return a pointer to the beginning of the image instance data.
		void const* data() const;

		/// Return a pointer of type genType which size must match the image format block size.
		template <typename genType>
		genType* data();

		/// Return a pointer of type genType which size must match the image format block size.
		template <typename genType>
		genType const* data() const;

		/// Clear the entire image storage_linear with zeros
		void clear();

		/// Clear the entire image storage_linear with Texel which type must match the image storage_linear format block size
		/// If the type of genType doesn't match the type of the image format, no conversion is performed and the data will be reinterpreted as if is was of the image format. 
		template <typename genType>
		void clear(genType const& Texel);

		/// Load the texel located at TexelCoord coordinates.
		/// It's an error to call this function if the format is compressed.
		/// It's an error if TexelCoord values aren't between [0, dimensions].
		template <typename genType>
		genType load(extent_type const& TexelCoord);

		/// Store the texel located at TexelCoord coordinates.
		/// It's an error to call this function if the format is compressed.
		/// It's an error if TexelCoord values aren't between [0, dimensions].
		template <typename genType>
		void store(extent_type const& TexelCoord, genType const& Data);

	private:
		/// Create an image object by sharing an existing image storage_linear from another image instance.
		/// This image object is effectively an image view where the layer, the face and the level allows identifying
		/// a specific subset of the image storage_linear source. 
		/// This image object is effectively a image view where the format can be reinterpreted
		/// with a different compatible image format.
		explicit image(
			std::shared_ptr<storage_linear> Storage,
			format_type Format,
			size_type BaseLayer,
			size_type BaseFace,
			size_type BaseLevel);

		std::shared_ptr<storage_linear> Storage;
		format_type const Format;
		size_type const BaseLevel;
		data_type* Data;
		size_type const Size;

		data_type* compute_data(size_type BaseLayer, size_type BaseFace, size_type BaseLevel);
		size_type compute_size(size_type Level) const;
	};
}//namespace gli

#include "./core/image.inl"
