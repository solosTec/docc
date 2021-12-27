/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOC2SITE_PAGE_H
#define DOC2SITE_PAGE_H

#include <cyng/obj/intrinsics/container.h>

#include <filesystem>
//#include <vector>
#include <string>
#include <map>

namespace docscript {

	struct page {
		page(std::string, std::string, std::string, bool, std::filesystem::path);

		std::string const title_;
		std::string const navbar_;
		std::string const footer_;
		bool const enabled_;
		std::filesystem::path const source_;
	};

	std::map<std::string, page> read_pages(cyng::vector_t);
}

#endif
