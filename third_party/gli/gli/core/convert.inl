#include "../core/convert_func.hpp"

namespace gli
{
	template <typename texture_type>
	inline texture_type convert(texture_type const& Texture, format Format)
	{
		typedef float T;
		typedef typename texture::extent_type extent_type;
		typedef typename texture_type::size_type size_type;
		typedef typename extent_type::value_type component_type;
		typedef typename detail::convert<texture_type, T, defaultp>::fetchFunc fetch_type;
		typedef typename detail::convert<texture_type, T, defaultp>::writeFunc write_type;

		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT(has_decoder(Texture.format()) || !is_compressed(Texture.format()));
		GLI_ASSERT(!is_compressed(Format));

		fetch_type Fetch = detail::convert<texture_type, T, defaultp>::call(Texture.format()).Fetch;
		write_type Write = detail::convert<texture_type, T, defaultp>::call(Format).Write;

		texture Storage(Texture.target(), Format, Texture.texture::extent(), Texture.layers(), Texture.faces(), Texture.levels(), Texture.swizzles());
		texture_type Copy(Storage);

		for(size_type Layer = 0; Layer < Texture.layers(); ++Layer)
		for(size_type Face = 0; Face < Texture.faces(); ++Face)
		for(size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			extent_type const& Dimensions = Texture.texture::extent(Level);

			for(component_type k = 0; k < Dimensions.z; ++k)
			for(component_type j = 0; j < Dimensions.y; ++j)
			for(component_type i = 0; i < Dimensions.x; ++i)
			{
				typename texture_type::extent_type const Texelcoord(extent_type(i, j, k));
				Write(
					Copy, Texelcoord, Layer, Face, Level,
					Fetch(Texture, Texelcoord, Layer, Face, Level));
			}
		}

		return texture_type(Copy);
	}

}//namespace gli

