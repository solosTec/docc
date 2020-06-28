/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 


#include "site.h"
#include "../../src/driver.h"
#include <html/dom.hpp>

#include <cyng/json.h>
#include <cyng/dom/reader.h>
#include <cyng/io/serializer.h>
#include <cyng/io/io_chrono.hpp>
#include <cyng/factory.h>
#include <cyng/set_cast.h>
#include <cyng/vector_cast.hpp>

#include <iostream>
#include <fstream>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace docscript
{

	std::pair<dom::element, bool> generate_dropdown_menu(dict_t const& dict
		, cyng::vector_t const&
		, std::string const& page);

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
		auto const name = cyng::value_cast<std::string>(reader["site"].get("name"), "example.com");
		auto const id = cyng::value_cast<std::string>(reader["site"].get("tag"), "89712b23-b8e4-479c-ae27-e6b76a41e090");
		auto const tag = boost::uuids::string_generator()(id);
		auto const index = cyng::value_cast<std::string>(reader["site"].get("index"), "home");
		auto const css = cyng::value_cast<std::string>(reader["site"].get("css"), "css/style.css");
		auto const version = cyng::value_cast<std::string>(reader["site"].get("bootstrap"), "4.5.0");
		auto const pages = cyng::vector_cast<std::string>(reader["site"].get("pages"), "");

		/**
		 * namespace for UUID generation based on SHA1
		 */
		boost::uuids::name_generator_sha1 name_gen(tag);

		//
		//	compile pages
		//
		if (reader.exists("pages"))
		{
			//
			//	generate unique files names for every page
			//
			auto const vec = cyng::to_vector(reader.get("pages"));
			auto const dict = create_page_dict(vec, index, name_gen, out);

			if (!reader.exists("menus"))
			{
				std::cerr << "***warning: no menus defined" << std::endl;
			}

			generate_pages(dict
				, pages
				, cyng::to_vector(reader.get("menus"))
				, name_gen
				, css
				, out);

			build_site(dict
				, pages
				, css
				, out);

		}
		else {
			std::cerr << "***warning: no pages defined" << std::endl;
		}
	}

	void site::build_site(dict_t const& dict
		, std::vector<std::string> const& site
		, std::string const& css
		, cyng::filesystem::path const& out)
	{
		for (auto const& p : site) {
			auto pos = dict.find(p);
			if (pos != dict.end()) {

				build_page(pos->second, css, out);
			}
		}
	}

	void site::build_page(page const& p
		, std::string const& css
		, cyng::filesystem::path const& out)
	{
		//
		//	build page
		//
		std::ofstream ofs((out / p.get_file()).string());
		if (ofs.is_open()) {

			std::cout 
				<< "***info: build page " 
				<< p.get_name()
				<< " from "
				<< p.get_fragment()
				<< std::endl;

			ofs
				<< "<!doctype html>"
				<< std::endl
				<< "<html lang=\""
				<< "en"	//	ToDo: language
				<< "\">"
				<< std::endl
				<< "<head>"
				<< std::endl
				<< "\t<meta charset=\"utf-8\" />"
				<< std::endl
				<< "\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">"
				<< std::endl
				<< "\t<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css\" integrity=\"sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk\" crossorigin=\"anonymous\">"
				<< std::endl
				;

			if (!css.empty()) {
				import_css(ofs, p, css, out);
			}
			if (p.has_css()) {

				ofs
					<< "\t<link rel=\"stylesheet\" href=\""
					<< p.get_css()
					<< "\">"
					<< std::endl
					;
			}
			ofs
				<< "\t<title>"
				<< p.get_title()
				<< "</title>"
				<< std::endl
				<< "</head>"
				<< std::endl
				<< "<body>"
				<< std::endl
				;

			import_menu(ofs, p, out);
			import_body(ofs, p, out);

			ofs
				<< std::endl
				<< "\t<script src=\"https://code.jquery.com/jquery-3.5.1.slim.min.js\" integrity=\"sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj\" crossorigin=\"anonymous\"></script>"
				<< std::endl
				<< "\t<script src=\"https://cdn.jsdelivr.net/npm/popper.js@1.16.0/dist/umd/popper.min.js\" integrity=\"sha384-Q6E9RHvbIyZFJoft+2mJbHaEWldlvI9IOYy5n3zV9zzTtmI3UksdQRVvoxMfooAo\" crossorigin=\"anonymous\"></script>"
				<< std::endl
				<< "\t<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/js/bootstrap.min.js\" integrity=\"sha384-OgVRvuATP1z7JjHLkuOU7Xw704+h835Lr+6QL9UvYjZE3Ipu6Tp75j7Bh/kR0JKI\" crossorigin=\"anonymous\"></script>"
				<< std::endl
				<< "</body>"
				<< std::endl
				<< "</html>"
				<< std::endl
				;
		}
	}

	void site::generate_pages(dict_t const& dict
		, std::vector<std::string> const& site
		, cyng::vector_t&& menus
		, boost::uuids::name_generator_sha1& gen
		, cyng::filesystem::path css_global
		, cyng::filesystem::path const& out)
	{
		for (auto const& p : site) {
			auto pos = dict.find(p);
			if (pos != dict.end()) {

				auto const obj = get_menu(menus, pos->second.get_menu());
				if (obj.is_null()) {

					std::cerr
						<< "***warning: menu [" 
						<< pos->second.get_menu() 
						<< "] not defined" 
						<< std::endl;
				}

				generate_page(dict, pos->second, obj, out);
			}
			else {
				std::cerr 
					<< "***error: page [" 
					<< p
					<< "] is not defined"
					<< std::endl;
			}
		}

	}

	void site::generate_page(dict_t const& dict
		, page const& p
		, cyng::object menu
		, cyng::filesystem::path const& out)
	{
		//
		//	Construct driver instance
		//
		driver d(includes_, verbose_);

		//
		//	generate some temporary file names for intermediate files
		//
		auto const tmp = cyng::filesystem::temp_directory_path() / (p.get_file().filename().string() + ".bin");

		//
		//	Start driver with the main/input file
		//
		d.generate_bootstrap_page(cyng::filesystem::path(p.get_source())
			, tmp
			, p.get_fragment());

		if (!menu.is_null()) {

			auto const reader = cyng::make_reader(menu);
			auto const menu_name = cyng::value_cast<std::string>(reader.get("name"), "");
			auto const placement = cyng::value_cast<std::string>(reader.get("placement"), "sticky-top");
			auto const color_scheme = cyng::value_cast<std::string>(reader.get("color-scheme"), "dark");
			auto const brand = cyng::value_cast<std::string>(reader.get("brand"), "images/logo.svg");

			std::cout 
				<< "***info: generate menu [" 
				<< menu_name 
				<< "] for page [" 
				<< p.get_name()
				<< "]"
				<< std::endl;

			generate_menu(dict, p.get_name(), p.get_tag(), brand, color_scheme, cyng::to_vector(reader.get("items")), out);

		}
	}

	void site::generate_menu(dict_t const& dict
		, std::string const& page
		, boost::uuids::uuid tag
		, std::string const& brand
		, std::string const& color_scheme
		, cyng::vector_t&& vec
		, cyng::filesystem::path const& out)
	{
		//
		//	test brand image
		//
		auto const r = resolve_path(includes_, brand);
		if (r.second) {

			//
			//	copy to build directory
			//
			cyng::filesystem::remove(out / brand);
			cyng::filesystem::copy(r.first, out / brand);
		}
		else {
			std::cerr << "***error: brand [" << brand << "] not found" << std::endl;

		}


		//
		//	get temporary file name
		//
		auto const p = dict.find(page)->second.get_nav();
		
		std::ofstream ofs(p.string());
		if (ofs.is_open()) {

			std::string cs = boost::algorithm::equals(color_scheme, "light")
				? "navbar navbar-expand-lg navbar-light bg-light"
				: boost::algorithm::equals(color_scheme, "primary")
					? "navbar navbar-expand-lg navbar-dark bg-primary"
					: boost::algorithm::equals(color_scheme, "success")
						? "navbar navbar-expand-lg navbar-dark bg-success"
						: boost::algorithm::equals(color_scheme, "secondary")
							? "navbar navbar-expand-lg navbar-dark bg-secondary"
							: boost::algorithm::equals(color_scheme, "custom")
								? "navbar navbar-expand-lg navbar-dark navbar-custom"
								: "navbar navbar-expand-lg navbar-dark bg-dark"
						;

			//
			//	generate menu items
			//
			auto ul = dom::ul(dom::class_("navbar-nav ml-auto"));
			for (auto const& obj : vec) {

				auto const reader = cyng::make_reader(obj);
				auto const title = cyng::value_cast<std::string>(reader.get("title"), "?");
				auto const ref = cyng::value_cast<std::string>(reader.get("ref"), "#");
				auto const items = cyng::to_vector(reader.get("items"));

				auto const pos = dict.find(ref);
				auto const enabled = pos != dict.end() && pos->second.is_enabled();
				auto const file = (pos != dict.end())
					? pos->second.get_file()
					: ref
					;

				if (items.empty()) {

					//	set active, enabled
					std::string class_li{ boost::algorithm::equals(page, ref) ? "nav-item active" : "nav-item" };
					std::string class_a{ enabled ? "nav-link" : "nav-link disabled"};

					ul += dom::li(dom::class_(class_li), 
						dom::a(dom::class_(class_a), dom::href_(file.string()), title)
					);
				}
				else {

					//
					//	active path?
					//
					std::pair<dom::element, bool> r = generate_dropdown_menu(dict, items, page);
					std::string class_li{ r.second ? "nav-item active dropdown" : "nav-item dropdown" };

					ul += dom::li(dom::class_(class_li)
						, dom::a(dom::class_("nav-link dropdown-toggle"), dom::href_("#"), dom::data_toggle_("dropdown"), title)
						, r.first
					);
				}
			}

			//
			//	generate menu
			//
			auto nav = dom::nav(dom::class_(cs), 
				dom::a(dom::class_("navbar-brand")
					, dom::href_("#")
					, dom::img(dom::src_(brand), dom::width_(30), dom::height_(30), dom::alt_("alt"), dom::loading_("lazy"))
				),
				dom::button(dom::class_("navbar-toggler")
					, dom::type_("button")
					, dom::data_toggle_("collapse")
					, dom::span(dom::class_("navbar-toggler-icon"))
				),
				dom::div(dom::class_("collapse navbar-collapse"), ul)
			);


			auto section = dom::section(dom::class_("container-fluid"), nav);
			ofs
				<< section(1)
				<< std::endl
				;
		}
		else {
			std::cerr << "***error: cannot open [" << p << "]" << std::endl;
		}
	}

	void site::import_css(std::ofstream& ofs, page const& p, std::string const& css, cyng::filesystem::path const& out)
	{
		//
		//	search css file
		//
		auto const r = resolve_path(includes_, css);
		if (r.second) {

			std::cerr
				<< "***info: copy ["
				<< r.first
				<< "] => ["
				<< (out / css)
				<< "]"
				<< std::endl;

			//
			//	copy to build directory
			//
			cyng::filesystem::remove(out / css);
			cyng::filesystem::copy(r.first, out / css);

			//
			//	insert link
			//
			ofs
				<< "\t<link rel=\"stylesheet\" href=\""
				<< css
				<< "\">"
				<< std::endl
				;
		}
		else {

			std::cerr 
				<< "***error: css file [" 
				<< css 
				<< "] not found" 
				<< std::endl;
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

	std::pair<dom::element, bool> generate_dropdown_menu(dict_t const& dict
		, cyng::vector_t const& vec
		, std::string const& page)
	{
		bool active{ false };
		auto div = dom::div(dom::class_("dropdown-menu"));
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(obj);
			auto const title = cyng::value_cast<std::string>(reader.get("title"), "?");
			auto const ref = cyng::value_cast<std::string>(reader.get("ref"), "#");
			auto const type = cyng::value_cast<std::string>(reader.get("type"), "item");
			//auto const items = cyng::to_vector(reader.get("items"));	//	ToDo. nested dropdown

			if (boost::algorithm::equals(type, "divider")) {
				div += dom::div(dom::class_("dropdown-divider"));
			}
			else {

				auto const pos = dict.find(ref);
				auto const enabled = pos != dict.end() && pos->second.is_enabled();
				auto const file = (pos != dict.end())
					? pos->second.get_file()
					: ref
					;

				std::string class_a{ enabled ? "dropdown-item" : "dropdown-item disabled" };
				div += dom::a(dom::class_(class_a), dom::href_(file.string()), title);

				//
				//	propagate upwards that this an active path
				//
				if (boost::algorithm::equals(page, ref)) {
					active = true;
				}
			}
		}
		return std::make_pair(div, active);
	}

	dict_t create_page_dict(cyng::vector_t const& pages
		, std::string const& index
		, boost::uuids::name_generator_sha1& gen
		, cyng::filesystem::path const& out)
	{
		dict_t	dict;

		for (auto& obj : pages) { 
			auto cp = cyng::object_cast<cyng::tuple_t>(obj);
			if (cp != nullptr) {

				auto const reader = cyng::make_reader(*cp);
				auto const name = cyng::value_cast<std::string>(reader.get("name"), "");
				auto const title = cyng::value_cast<std::string>(reader.get("title"), name);

				auto const source = cyng::value_cast(reader.get("name"), name);
				auto const tag = gen(cyng::value_cast(reader.get("tag"), source));
				auto const id = boost::uuids::to_string(tag);

				auto const enabled = cyng::value_cast(reader.get("enabled"), false);
				auto const type = cyng::value_cast<std::string>(reader.get("type"), "page");
				auto const css_page = cyng::value_cast<std::string>(reader.get("css"), "");
				auto const menu = cyng::value_cast<std::string>(reader.get("menu"), "");
				auto const footer = cyng::value_cast<std::string>(reader.get("footer"), "");

				//index
				std::string const file = (boost::algorithm::equals(index, name))
					? "index.html"
					: (id + ".html")
					;

				dict.emplace(name, page(name
					, title
					, enabled
					, source
					, tag
					, file
					, out / ("page-" + id + ".html")
					, type
					, css_page
					, menu
					, out / ("menu-" + id + ".html")
					, footer));

				std::cout << "***info: [" << name << "] => " << (id + ".html") << std::endl;

			}
		}

		return dict;
	}

	void import_menu(std::ofstream& ofs, page const& p, cyng::filesystem::path const& out)
	{
		if (p.has_menu()) {

			std::ifstream ifs(p.get_nav().string());
			if (ifs.is_open()) {
				ofs << ifs.rdbuf();
				ifs.close();
				cyng::filesystem::remove(p.get_nav());
			}
			else {
				std::cout << "***warning: cannot read [" << p.get_nav() << std::endl;

			}
		}
	}

	void import_body(std::ofstream& ofs, page const& p, cyng::filesystem::path const& out)
	{
		std::ifstream ifs(p.get_fragment().string());
		if (ifs.is_open()) {
			ofs << ifs.rdbuf();
			ifs.close();
			cyng::filesystem::remove(p.get_fragment());
		}
		else {
			std::cout << "***warning: cannot read [" << (out / p.get_fragment()) << std::endl;
		}
	}


}
