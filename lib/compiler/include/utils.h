/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_UTILS_H
#define DOCC_SCRIPT_UTILS_H

#include <filesystem>
#include <vector>

namespace docscript {

    /**
     * @param p path
     * @param ext extension
     * @return path with extension
     */
    std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext);

	/**
	 * Scan all provided directories for p
	 */
	std::pair<std::filesystem::path, bool> resolve_path(std::vector< std::filesystem::path >const& inc, std::filesystem::path p);

    std::vector<std::filesystem::path> get_include_paths(std::vector<std::string> const& vec, std::filesystem::path parent_path);

    //
    //	set console outpt code page to UTF-8
    //	requires a TrueType font like Lucida 
    //
    void init_console();

}

#endif
