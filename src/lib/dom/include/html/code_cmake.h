/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_CODE_CMAKE_H
#define HTML_CODE_CMAKE_H

#include <cyng/io/parser/stream.hpp>

namespace dom
{
	void cmake_to_html(std::ostream& os,
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> start,
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> end,
		bool numbers);
}

#endif
