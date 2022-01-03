/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOC2SITE_BUILD_H
#define DOC2SITE_BUILD_H

#include <cyng/obj/intrinsics/container.h>

#include <filesystem>
#include <vector>
#include <chrono>

#include <boost/uuid/uuid.hpp>


namespace cyng {
	class mesh;
}

namespace docscript {

	struct page;
	struct footer;
	struct navbar;
	class build
	{
	public:
		build(boost::uuids::uuid tag
			, std::vector<std::filesystem::path> inc
			, std::filesystem::path out
			, std::filesystem::path cache
			, std::filesystem::path bs
			, int verbose
			, std::string const& locale
			, std::string const& country
			, std::string const& language
			, std::string const& encoding);

		int run(std::size_t, std::filesystem::path working_dir);

	private:
		int run(std::size_t pool_size, 
			cyng::param_map_t site,
			cyng::vector_t pages,
			cyng::vector_t footers,
			cyng::vector_t navbars,
			cyng::object downloads,
			std::vector<std::string> languages);

		void generate_page(cyng::mesh&, std::string const& name, page const&, footer const&, navbar const&);
		void finalize_page(std::string const& name, std::string const& file_name, cyng::param_map_t& meta);
		void emit_header(std::ostream&, cyng::param_map_t& meta);

	private:
		boost::uuids::uuid const tag_;
		std::vector<std::filesystem::path> const inc_;
		std::filesystem::path out_dir_;
		std::filesystem::path cache_dir_;
		std::filesystem::path bs_dir_;
		int const verbose_;
		std::string const& locale_;
		std::string const& country_;
		std::string const& language_;
		std::string const& encoding_;

	};
}

#endif
