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
#include <chrono>

namespace docscript {

	class initialize
	{
	public:
		initialize(std::vector<std::filesystem::path> inc
			, int verbose);

		int run(std::filesystem::path working_dir);

	private:
		cyng::tuple_t generate_control(std::tm const& tm) const;
		cyng::tuple_t generate_page_home() const;
		cyng::tuple_t generate_navbar_main() const;
		cyng::tuple_t generate_footer_main(std::tm const& tm) const;
		cyng::tuple_t generate_downloads() const;

		void generate_logo(std::filesystem::path);

		void download_bootstrap(std::filesystem::path, std::string v) const;

	private:
		std::vector<std::filesystem::path> const inc_;
		int const verbose_;
	};
}

#endif
