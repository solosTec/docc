/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_INCLUDE_H
#define DOCSCRIPT_INCLUDE_H

#include <cyng/compatibility/file_system.hpp>
#include <tuple>

namespace docscript
{
	/**
	 * contains the following information about an include file:
	 * <ol>
	 * <li>file to include</li>
	 * <li>line to start</li>
	 * <li>length of range to include</li>
	 * </ol>
	 */
	using incl_t = std::tuple<cyng::filesystem::path, std::size_t, std::size_t>;
}

#endif	




