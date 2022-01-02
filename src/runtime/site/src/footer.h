/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOC2SITE_FOOTER_H
#define DOC2SITE_FOOTER_H

#include <cyng/obj/intrinsics/container.h>

#include <filesystem>
#include <string>
#include <map>

namespace docscript {

	struct footer {
		footer(std::string, std::string, bool);

		std::string const bg_color_;
		std::string const content_;
		bool const enabled_;
	};

	std::map<std::string, footer> read_footers(cyng::vector_t);
}

#endif
