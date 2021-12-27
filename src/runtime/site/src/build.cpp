
#include <build.h>
#include <site_defs.h>
#include <page.h>
#include <footer.h>
#include <navbar.h>

#include <docc/context.h>
#include <docc/utils.h>

#include <cyng/sys/locale.h>
#include <cyng/obj/container_factory.hpp>
#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/vector_cast.hpp>
#include <cyng/obj/algorithm/reader.hpp>
#include <cyng/io/serialize.h>
#include <cyng/parse/json.h>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


namespace docscript {
	build::build(std::vector<std::filesystem::path> inc
		, std::filesystem::path const& out
		, std::filesystem::path const& cache
		, std::filesystem::path const& bs
		, int verbose
		, std::string const& locale
		, std::string const& country
		, std::string const& language
		, std::string const& encoding)
	: inc_(inc)
		, out_dir_(out)
		, cache_dir_(cache)
		, bs_dir_(bs)
		, verbose_(verbose)
		, locale_(locale)
		, country_(country)
		, language_(language)
		, encoding_(encoding)
	{}

	int build::run(std::filesystem::path ctrl_file) {

		//
		//	read control file
		//
		auto const obj = cyng::json::parse_file(ctrl_file.string());
		if (obj) {
			auto pm = cyng::container_cast<cyng::param_map_t>(obj);

			if (verbose_ > 2) {
				fmt::print(
					stdout,
					fg(fmt::color::gray),
					"***info : control file [{}] has {} entries\n", ctrl_file.string(), pm.size());
			}

			auto const reader = cyng::make_reader(pm);
			
			return run(
				cyng::container_cast<cyng::param_map_t>(reader.get("site")),
				cyng::container_cast<cyng::vector_t>(reader.get("pages")),
				cyng::container_cast<cyng::vector_t>(reader.get("footers")),
				cyng::container_cast<cyng::vector_t>(reader.get("navbars")),
				reader.get("downloads"),
				cyng::vector_cast<std::string>(reader.get("languages"), "en"));
		}

		fmt::print(
			stdout,
			fg(fmt::color::dark_orange) | fmt::emphasis::bold,
			"***warn  : config file [{}] not found\n", ctrl_file.string());
		return EXIT_FAILURE;

	}

	int build::run(cyng::param_map_t site,
		cyng::vector_t p,
		cyng::vector_t f,
		cyng::vector_t nb,
		cyng::object downloads,
		std::vector<std::string> languages) {

		auto const pages = read_pages(p);
		auto const footers = read_footers(f);
		auto const navbars = read_navbars(nb);

		auto const reader = cyng::make_reader(site);
		auto const site_name = reader.get("name", "name");
		auto const site_index = reader.get("index", "index");
		auto const site_css = reader.get("css", "css");
		auto const site_pages = cyng::vector_cast<std::string>(reader.get("pages"), "home");
		if (verbose_ > 2) {
			fmt::print(
				stdout,
				fg(fmt::color::gray),
				"***info : {} page(s) configured\n", site_pages.size());
		}
		for (auto const& page : site_pages) {
			if (verbose_ > 2) {
				fmt::print(
					stdout,
					fg(fmt::color::gray),
					"***info : generate page {}\n", page);
			}

			auto const pos_page = pages.find(page);
			if (pos_page != pages.end()) {
				auto const pos_footer = footers.find(pos_page->second.footer_);
				if (pos_footer != footers.end()) {

					auto pos_navbar = navbars.find(pos_page->second.navbar_);
					if (pos_navbar != navbars.end()) {
						generate_page(page, pos_page->second, pos_footer->second, pos_navbar->second);
					}
					else {
						fmt::print(
							stdout,
							fg(fmt::color::dark_orange) | fmt::emphasis::bold,
							"***warn  : navbar {} in page [{}] is not configured\n", pos_page->second.navbar_, page);

					}
				}
				else {
					fmt::print(
						stdout,
						fg(fmt::color::dark_orange) | fmt::emphasis::bold,
						"***warn  : footer {} in page [{}] is not configured\n", pos_page->second.footer_, page);
				}
			}
			else {
				fmt::print(
					stdout,
					fg(fmt::color::dark_orange) | fmt::emphasis::bold,
					"***warn  : page [{}] is not configured\n", page);
			}
		}

		return EXIT_SUCCESS;
	}

	void build::generate_page(std::string const& name, page const& cfg_page, footer const& cfg_footer, navbar const& cfg_navbar) {

		//
		//	create context
		//
		context ctx(verify_extension(cache_dir_ / name, "docs"), inc_, verbose_);

		//
		//	check source file
		//
		auto const r = ctx.lookup(name, "docscript");
		if (r.second) {

		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : source file [{}] not found\n", r.first.string());
		}
	}
}
