/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_CODE_CURLY_H
#define HTML_CODE_CURLY_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>
#include <cyng/io/parser/stream.hpp>

#include <filesystem>
#include <iostream>
#include <chrono>
#include <fstream>

namespace dom
{
	void curly_to_html(std::ostream&
		, cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>>
		, cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>>
		, bool numbers
		, std::string const& lang);
}

#endif
