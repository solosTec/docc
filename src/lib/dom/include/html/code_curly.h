/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_CODE_CURLY_H
#define HTML_CODE_CURLY_H

#include <cyng/io/parser/stream.hpp>

namespace dom
{
	void curly_to_html(std::ostream&
		, cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>>
		, cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>>
		, bool numbers
		, std::string const& lang);
}

#endif
