
#include <build.h>
#include <site_defs.h>
#include <page.h>
#include <footer.h>
#include <navbar.h>

#include <docc/context.h>
#include <docc/utils.h>
#include <docc/reader.h>
#include <asm/reader.h>


#include <cyng/sys/locale.h>
#include <cyng/obj/container_factory.hpp>
#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/vector_cast.hpp>
#include <cyng/obj/algorithm/reader.hpp>
#include <cyng/io/serialize.h>
#include <cyng/parse/json.h>
#include <cyng/task/controller.h>
#include <cyng/vm/mesh.h>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


namespace docscript {
	build::build(std::vector<std::filesystem::path> inc
		, std::filesystem::path out
		, std::filesystem::path cache
		, std::filesystem::path bs
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

	int build::run(std::size_t pool_size, std::filesystem::path ctrl_file) {

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
				pool_size,
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

	int build::run(std::size_t pool_size, 
		cyng::param_map_t site,
		cyng::vector_t p,
		cyng::vector_t f,
		cyng::vector_t nb,
		cyng::object downloads,
		std::vector<std::string> languages) {

		cyng::controller ctl(pool_size);
		cyng::mesh fabric(ctl);

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
						generate_page(fabric, page, pos_page->second, pos_footer->second, pos_navbar->second);
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

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		ctl.cancel();
		ctl.stop();

		return EXIT_SUCCESS;
	}

	void build::generate_page(cyng::mesh& fabric, std::string const& name, page const& cfg_page, footer const& cfg_footer, navbar const& cfg_navbar) {

		//
		//	create context
		//
		context ctx(verify_extension(cache_dir_ / name, "docs"), inc_, verbose_);

		//
		//	check input file
		//
		auto const r = ctx.lookup(name, "docscript");
		if (r.second) {

			//
			//	start compiler and generate an assembler file
			//
			docscript::reader compiler(ctx);
			compiler.read(r.first);

			if (ctx.get_verbosity(2)) {

				fmt::print(stdout, fg(fmt::color::forest_green),
					"***info : intermediate file {} complete\n",
					ctx.get_output_path());
			}

			//
			//	generate program from assembler
			//
			docasm::reader assembler(std::filesystem::path(ctx.get_output_path()).replace_extension("cyng"), inc_, verbose_);
			assembler.read(ctx.get_output_path());

			//
			//	load and execute program
			//
			std::ifstream ifs(assembler.get_output_path().string(),	std::ios::binary);
			if (ifs.is_open()) {
				ifs.unsetf(std::ios::skipws);

				//
				//	open output file
				// 
				auto const output_file = verify_extension(out_dir_ / name, "html").string();
				std::ofstream ofs(output_file, std::ios::trunc);
				if (ofs.is_open()) {

					//
					//	navbar
					// 
					emit_navbar(ofs, cfg_navbar, cfg_page);

					//
					//	content
					// 
					//emit_page(ofs, fabric, cfg_page);

					//
					//	footer
					// 
					emit_footer(ofs, cfg_footer);
				}
				else {
					fmt::print(stdout,
						fg(fmt::color::dark_orange) | fmt::emphasis::bold,
						"***info : cannot open output file [{}]\n", output_file);
				}
			}
			else {
				fmt::print(stdout,
					fg(fmt::color::dark_orange) | fmt::emphasis::bold,
					"***info : assembler output [{}] not found\n", assembler.get_output_path().string());
			}
		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : source file [{}] not found\n", r.first.string());
		}
	}

	void build::emit_navbar(std::ostream& os, navbar const& nb, page const& p) {
		os << "<nav class = \"navbar navbar-dark bg-dark\">\n";
		os << "</nav>\n";
	}

	void build::emit_footer(std::ostream& os, footer const& f) {
		os << "<footer class=\"footer mt-auto py-3 " << f.bg_color_ << "\">\n";
		os << "\t<div class=\"container\">\n";
		os << "\t\t<span class=\"text-muted\">" << f.content_ << "</span>\n";
		os << "\t</div>\n";
		os << "</footer>\n";
	}

}
