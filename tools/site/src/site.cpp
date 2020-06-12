/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 


#include "site.h"
#include "../../src/driver.h"

#include <cyng/json.h>
#include <cyng/dom/reader.h>
#include <cyng/io/serializer.h>
#include <cyng/io/io_chrono.hpp>
#include <cyng/factory.h>
#include <cyng/set_cast.h>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace docscript
{

	site::site(std::vector< std::string >const& inc
		, int verbose)
	: includes_(inc.begin(), inc.end())
		, verbose_(verbose)
	{}

	site::~site()
	{}

	int site::run(cyng::filesystem::path const& cfg
		, cyng::filesystem::path const& out
		, bool gen_robot
		, bool gen_sitemap)
	{

		if (cyng::filesystem::exists(cfg) && cyng::filesystem::is_regular_file(cfg)) {

			std::cout << "***info: use configuration file " << cfg << std::endl;

			//
			//	read configuration file
			//
			auto const config = cyng::json::read_file(cfg.string());
			if (verbose_ > 8) {
				std::cout
					<< cyng::io::to_str(config)
					<< std::endl;
			}

			//
			//	generate site
			//
			if (!config.is_null()) {

				auto const tpl = cyng::to_tuple(config);
				generate(cyng::to_param_map(tpl), out);

				//
				//	generate index file
				//
				//	generate_index(out, gen_robot, gen_sitemap);
				return EXIT_SUCCESS;
			}

			std::cerr << "***error: cannot parse configuration file " << cfg << std::endl;
			return EXIT_FAILURE;
		}

		//
		//	not found
		//
		std::cerr << "***error: " << cfg << " not found" << std::endl;
		return EXIT_FAILURE;
	}

	void site::generate(cyng::param_map_t&& cfg, cyng::filesystem::path const& out)
	{
		auto const reader = cyng::make_reader(cfg);

		//
		//	compile pages
		//
		if (reader.exists("pages"))
		{
			generate_pages(cyng::to_vector(reader.get("pages")), out);
		}
		else {
			std::cerr << "***warning: no pages defined" << std::endl;
		}

		//
		//	generate menues
		//
		if (reader.exists("menues"))
		{
			generate_menues(cyng::to_vector(reader.get("menues")), out);
		}
		else {
			std::cerr << "***warning: no menues defined" << std::endl;
		}
	}

	void site::generate_pages(cyng::vector_t&& vec, cyng::filesystem::path const&)
	{
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(obj);
			auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
			std::cout << "***info: generate page " << name << std::endl;
		}
	}

	void site::generate_menues(cyng::vector_t&& vec, cyng::filesystem::path const&)
	{
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(obj);
			auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
			std::cout << "***info: generate menu " << name << std::endl;
		}
	}

}
