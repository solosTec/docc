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

#include <filesystem>
#include <iostream>
#include <chrono>
#include <fstream>

namespace dom
{
	void curly_to_html(std::ostream&, std::ifstream& ifs, bool numbers, std::string const& lang);
}

#endif
