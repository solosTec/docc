/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOC2SITE_INITIALIZE_H
#define DOC2SITE_INITIALIZE_H

#include <cyng/obj/intrinsics/container.h>

#include <filesystem>
#include <vector>

namespace docscript {

	class initialize
	{
	public:
		initialize(std::vector<std::filesystem::path> inc
			, int verbose);

		int run(std::filesystem::path working_dir);

	private:
		cyng::tuple_t generate_control() const;
		cyng::tuple_t generate_page_home() const;
		cyng::tuple_t generate_menu_main() const;

	private:
		std::vector<std::filesystem::path> const inc_;
		int const verbose_;
	};
}

#endif
