/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 


#include "site.h"
//#include "../../src/driver.h"
#include <html/node.hpp>

#include <cyng/json.h>
#include <cyng/dom/reader.h>
#include <cyng/io/serializer.h>
#include <cyng/io/io_chrono.hpp>
#include <cyng/factory.h>
#include <cyng/set_cast.h>

#include <iostream>
#include <fstream>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace docscript
{

	site::site(std::vector< std::string >const& inc
		, int verbose)
	: includes_(inc.begin(), inc.end())
		, verbose_(verbose)
		, uuid_gen_()
		, name_gen_(uuid_gen_())
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
		if (reader.exists("menus"))
		{
			generate_menues(cyng::to_vector(reader.get("menus")), out);
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
			auto const enabled = cyng::value_cast(reader.get("enabled"), false);
			if (enabled) {

				auto const source = cyng::value_cast(reader.get("name"), name);
				auto const tag = name_gen_(cyng::value_cast(reader.get("tag"), source));
				auto const id = boost::uuids::to_string(tag);

				std::cout << "***info: generate page [" << name << "]" << std::endl;
			}
			else {
				std::cout << "***warning: skip page  [" << name << "]" << std::endl;
			}
		}
	}

	void site::generate_menues(cyng::vector_t&& vec, cyng::filesystem::path const& out)
	{
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(obj);
			auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
			auto const enabled = cyng::value_cast(reader.get("enabled"), false);

			if (enabled) {

				auto const placement = cyng::value_cast<std::string>(reader.get("placement"), "sticky-top");
				auto const color_scheme = cyng::value_cast<std::string>(reader.get("color-scheme"), "dark");
				auto const brand = cyng::value_cast<std::string>(reader.get("brand"), "images/logo.png");
				auto const tag = name_gen_(cyng::value_cast(reader.get("tag"), name));

				std::cout << "***info: generate menu [" << name << "]" << std::endl;

				generate_menu(name, tag, brand, color_scheme, cyng::to_vector(reader.get("items")), out);
			}
			else {
				std::cout << "***warning: skip menu [" << name << "]" << std::endl;
			}
		}
	}

	void site::generate_menu(std::string const& name
		, boost::uuids::uuid tag
		, std::string const& brand
		, std::string const& color_scheme
		, cyng::vector_t&& vec
		, cyng::filesystem::path const& out)
	{
		auto const id = boost::uuids::to_string(tag);
		auto const p = out / (id + ".menu");
		
		std::ofstream of(p.string());
		if (of.is_open()) {

			std::string cs = boost::algorithm::equals(color_scheme, "light")
				? "navbar navbar-expand-lg navbar-light bg-light"
				: boost::algorithm::equals(color_scheme, "primary")
					? "navbar navbar-expand-lg navbar-dark bg-primary"
					: boost::algorithm::equals(color_scheme, "success")
						? "navbar navbar-expand-lg navbar-dark bg-success"
						: "navbar navbar-expand-lg navbar-dark bg-dark"
						;

			//
			//	generate menu
			//
			auto const nav = html::nav(html::class_(cs), 
				html::a(html::class_("navbar-brand")
					, html::href_("#")
					, html::img(html::src_("logo.svg"), html::width_(30), html::height_(30), html::alt_(""), html::loading_("lazy"))
				)
			);

			//
			//	generate menu items
			//
			for (auto const& obj : vec) {
				std::cout << "[" << cyng::io::to_str(obj) << "] " << p.generic_string() << std::endl;
			}

			of
				<< nav.to_str()
				<< std::endl
				;
		}
		else {
			std::cout << "***error: cannot open [" << p << "]" << std::endl;
		}
	}

}
