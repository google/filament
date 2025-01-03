#include "../save_dds.hpp"
#include "../save_kmg.hpp"
#include "../save_ktx.hpp"

namespace gli
{
	inline bool save(texture const & Texture, char const * Path)
	{
		return save(Texture, std::string(Path));
	}

	inline bool save(texture const & Texture, std::string const & Path)
	{
		if(Path.rfind(".dds") != std::string::npos)
			return save_dds(Texture, Path);
		if(Path.rfind(".kmg") != std::string::npos)
			return save_kmg(Texture, Path);
		if(Path.rfind(".ktx") != std::string::npos)
			return save_ktx(Texture, Path);
		return false;
	}
}//namespace gli
