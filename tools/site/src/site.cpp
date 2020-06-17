/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 


#include "site.h"
#include "../../src/driver.h"
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
#include <boost/uuid/uuid_generators.hpp>

namespace docscript
{

	html::node generate_dropdown_menu(cyng::vector_t const&);

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
		auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
		auto const id = cyng::value_cast<std::string>(reader.get("tag"), "89712b23-b8e4-479c-ae27-e6b76a41e090");
		auto const tag = boost::uuids::string_generator()(id);
		auto const css = cyng::value_cast<std::string>(reader.get("css"), "css/style.css");

		/**
		 * namespace for UUID generation based on SHA1
		 */
		boost::uuids::name_generator_sha1 name_gen(tag);

		//
		//	compile pages
		//
		if (reader.exists("pages"))
		{
			if (!reader.exists("menus"))
			{
				std::cerr << "***warning: no menues defined" << std::endl;
			}

			generate_pages(cyng::to_vector(reader.get("pages"))
				, cyng::to_vector(reader.get("menus"))
				, name_gen
				, css
				, out);
		}
		else {
			std::cerr << "***warning: no pages defined" << std::endl;
		}

		//
		//	generate menues
		//
		//if (reader.exists("menus"))
		//{
		//	generate_menus(cyng::to_vector(reader.get("menus")), name_gen, out);
		//}
		//else {
		//	std::cerr << "***warning: no menues defined" << std::endl;
		//}
	}

	void site::generate_pages(cyng::vector_t&& vec
		, cyng::vector_t&& menus
		, boost::uuids::name_generator_sha1& gen
		, cyng::filesystem::path css_global
		, cyng::filesystem::path const& out)
	{
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(obj);
			auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
			auto const enabled = cyng::value_cast(reader.get("enabled"), false);
			if (enabled) {

				auto const source = cyng::value_cast(reader.get("name"), name);
				auto const tag = gen(cyng::value_cast(reader.get("tag"), source));
				//auto const id = boost::uuids::to_string(tag);
				auto const type = cyng::value_cast<std::string>(reader.get("type"), "page");
				auto const css_page = cyng::value_cast<std::string>(reader.get("css"), "");
				auto const menu = cyng::value_cast<std::string>(reader.get("menu"), "");
				auto const footer = cyng::value_cast<std::string>(reader.get("footer"), "");

				std::cout << "***info: generate page [" << name << "]" << std::endl;
				auto const obj = get_menu(menus, menu);
				if (obj.is_null()) {
					std::cout << "***warning: menu [" << menu << "] not defined" << std::endl;
				}

				generate_page(name, tag, source, type, css_global, css_page, obj, footer, out);
			}
			else {
				std::cout << "***warning: skip page  [" << name << "]" << std::endl;
			}
		}
	}

	void site::generate_page(std::string const& name
		, boost::uuids::uuid tag
		, cyng::filesystem::path source
		, std::string const& type
		, cyng::filesystem::path css_global
		, cyng::filesystem::path css_page
		, cyng::object menu
		, std::string const& footer
		, cyng::filesystem::path const& out)
	{
		auto const id = boost::uuids::to_string(tag);
		auto const p = out / (id + ".page.html");

		//
		//	Construct driver instance
		//
		driver d(includes_, verbose_);

		//
		//	generate some temporary file names for intermediate files
		//
		cyng::filesystem::path tmp = cyng::filesystem::temp_directory_path() / (cyng::filesystem::path(source).filename().string() + ".bin");

		//
		//	Start driver with the main/input file
		//
		d.run(cyng::filesystem::path(source).filename()
			, tmp
			, p
			, true	//	only HTML body
			, false	//	generate meta data
			, false	//	index
			, "article");

		if (!menu.is_null()) {
			auto const reader = cyng::make_reader(menu);
			auto const menu_name = cyng::value_cast<std::string>(reader.get("name"), "");
			auto const placement = cyng::value_cast<std::string>(reader.get("placement"), "sticky-top");
			auto const color_scheme = cyng::value_cast<std::string>(reader.get("color-scheme"), "dark");
			auto const brand = cyng::value_cast<std::string>(reader.get("brand"), "images/logo.png");

			std::cout << "***info: generate menu [" << menu_name << "]" << std::endl;

			generate_menu(menu_name, tag, brand, color_scheme, cyng::to_vector(reader.get("items")), out);

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
		auto const p = out / (id + ".menu.html");
		
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
			//	generate menu items
			//
			//auto div = html::div(html::class_("collapse navbar-collapse")
			//	, html::ul(html::class_("navbar-nav mr-auto"))
			//);
			auto ul = html::ul(html::class_("navbar-nav mr-auto"));
			for (auto const& obj : vec) {

				auto const reader = cyng::make_reader(obj);
				auto const title = cyng::value_cast<std::string>(reader.get("title"), "?");
				auto const ref = cyng::value_cast<std::string>(reader.get("ref"), "#");
				auto const enabled = cyng::value_cast(reader.get("enabled"), false);
				auto const items = cyng::to_vector(reader.get("items"));

				if (items.empty()) {
					if (enabled) {
						ul += html::li(html::class_("nav-item"), 	//	ToDo: set active, enabled
							html::a(html::class_("nav-link"), html::href_(ref), title)
						);
					}
					else {
						ul += html::li(html::class_("nav-item"), 
							html::a(html::class_("nav-link disabled"), html::href_(ref), title)
						);
					}
				}
				else {
					ul += html::li(html::class_("nav-item dropdown")
						, html::a(html::class_("nav-link dropdown-toggle"), html::href_("#"), html::data_toggle_("dropdown"), title)
						, generate_dropdown_menu(items)
					);
				}
			}

			//
			//	generate menu
			//
			auto nav = html::nav(html::class_(cs), 
				html::a(html::class_("navbar-brand")
					, html::href_("#")
					, html::img(html::src_("logo.svg"), html::width_(30), html::height_(30), html::alt_(""), html::loading_("lazy"))
				),
				html::button(html::class_("navbar-toggler")
					, html::type_("button")
					, html::data_toggle_("collapse")
					, html::span(html::class_("navbar-toggler-icon"))
				),
				html::div(html::class_("collapse navbar-collapse"), ul)
			);


			of
				<< nav.to_str()
				<< std::endl
				;
		}
		else {
			std::cout << "***error: cannot open [" << p << "]" << std::endl;
		}
	}

	cyng::object get_menu(cyng::vector_t const& menus, std::string const& menu)
	{
		for (auto const& obj : menus) {
			auto const reader = cyng::make_reader(obj);
			auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
			if (boost::algorithm::equals(name, menu))	return obj;
		}
		return cyng::make_object();
	}

	cyng::object get_page(cyng::vector_t const& pages, std::string const& page)
	{
		for (auto const& obj : pages) {
			auto const reader = cyng::make_reader(obj);
			auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
			if (boost::algorithm::equals(name, page))	return obj;
		}
		return cyng::make_object();
	}

	html::node generate_dropdown_menu(cyng::vector_t const& vec)
	{
		auto div = html::div(html::class_("dropdown-menu"));
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(obj);
			auto const title = cyng::value_cast<std::string>(reader.get("title"), "?");
			auto const ref = cyng::value_cast<std::string>(reader.get("ref"), "#");
			auto const type = cyng::value_cast<std::string>(reader.get("type"), "item");
			//auto const items = cyng::to_vector(reader.get("items"));

			if (boost::algorithm::equals(type, "divider")) {
				div += html::div(html::class_("dropdown-divider"));
			}
			else {
				div += html::a(html::class_("dropdown-item"), html::href_(ref), title);
			}
		}
		return div;
	}

}
