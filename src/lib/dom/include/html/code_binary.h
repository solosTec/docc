/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_CODE_BINARY_H
#define HTML_CODE_BINARY_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>

#include <filesystem>
#include <iostream>
#include <chrono>
#include <fstream>

namespace dom
{
	void binary_to_html(std::ostream& os, std::istream_iterator<char> start, std::istream_iterator<char> end, bool numbers);
}

#endif
