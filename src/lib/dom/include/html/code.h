/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_CODE_H
#define HTML_CODE_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>

#include <filesystem>
#include <iostream>
#include <chrono>
#include <fstream>

namespace dom
{
	void code_to_html(std::ostream&, std::filesystem::path const&, std::string const& lang, bool numbers, std::string const& caption);
}

#endif
