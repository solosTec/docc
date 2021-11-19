#include <docc/utils.h>

#include <iostream>

#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#include "Windows.h"
#endif

namespace docscript {
    std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext)
	{
		if (!p.has_extension())	{
			p.replace_extension(ext);
		}
		return p;	
	}

	std::pair<std::filesystem::path, bool> resolve_path(std::vector< std::filesystem::path >const& inc, std::filesystem::path p)
	{
		//
		//	search all include paths for the specified file path
		//
		for (auto const& dir : inc)	{
			if (std::filesystem::exists(dir / p))	return std::make_pair((dir / p), true);
		}

		//
		//	not found - try harder
		//	search all include paths for the specified file name
		//
		for (auto const& dir : inc) {
			//	ignore path
			if (std::filesystem::exists(dir / p.filename()))	return std::make_pair((dir / p.filename()), true);
		}

		return std::make_pair(p, false);

	}

	std::vector<std::filesystem::path> get_include_paths(std::vector<std::string> const& vec, std::filesystem::path parent_path) {

		//
		//  convert from string to path
		//
		std::vector<std::filesystem::path> includes(vec.begin(), vec.end());

		//
		//	Add the path of the input file as include path, if it is not already specified
		//
		auto pos = std::find(vec.begin(), vec.end(), parent_path);
		if (pos == vec.end() && !parent_path.empty()) {
			includes.push_back(parent_path);
		}

		//
		//	last entry is empty
		//
#if BOOST_OS_WINDOWS
		includes.push_back(".\\");
#else
		includes.push_back("./");
#endif

		return includes;

	}

#if BOOST_OS_WINDOWS
	//
	//	set console outpt code page to UTF-8
	//	requires a TrueType font like Lucida 
	//
	void init_console() {
		if (::SetConsoleOutputCP(65001) == 0)
		{
			std::cerr
				<< "Cannot set console code page"
				<< std::endl
				;

		}
		auto h_out = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (h_out != INVALID_HANDLE_VALUE) {
			DWORD dwMode = 0;
			if (::GetConsoleMode(h_out, &dwMode)) {
				::SetConsoleMode(h_out, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
			}
		}
	}
#else
	void init_console() {
	}
#endif

}