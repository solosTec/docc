#include <utils.h>
//#include <docc.h>

namespace docscript {
    std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext)
	{
		if (!p.has_extension())
		{
			p.replace_extension(ext);
		}
		return p;	
	}

	std::pair<std::filesystem::path, bool> resolve_path(std::vector< std::filesystem::path >const& inc, std::filesystem::path p)
	{
		//
		//	search all include paths for the specified file path
		//
		for (auto const& dir : inc)
		{
			if (std::filesystem::exists(dir / p))	return std::make_pair((dir / p), true);
		}

		//
		//	not found - try harder
		//	search all include paths for the specified file name
		//
		for (auto const& dir : inc)
		{
			//	ignore path
			if (std::filesystem::exists(dir / p.filename()))	return std::make_pair((dir / p.filename()), true);
		}

		return std::make_pair(p, false);

	}

}