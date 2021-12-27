/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOC2SITE_NAVBAR_H
#define DOC2SITE_NAVBAR_H

#include <cyng/obj/intrinsics/container.h>

#include <filesystem>
#include <string>
#include <map>

namespace docscript {

	struct navbar {
		navbar(std::string, std::string, std::filesystem::path);

		std::string const placement_;
		std::string const color_scheme_;
		std::filesystem::path const brand_;	//	logo
	};

	std::map<std::string, navbar> read_navbars(cyng::vector_t);
}

#endif
