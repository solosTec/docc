
#include <initialize.h>
#include <site_defs.h>

#include <cyng/sys/locale.h>
#include <cyng/obj/container_factory.hpp>
#include <cyng/io/serialize.h>

#include <fstream>
#include <iostream>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


namespace docscript {
	initialize::initialize(std::vector<std::filesystem::path> inc
		, int verbose)
	: inc_(inc)
		, verbose_(verbose)
	{}

	int initialize::run(std::filesystem::path working_dir) {

		boost::uuids::random_generator uuid_gen; //	basic_random_generator<mt19937>

		//
		//	1. check for .doc2site subdirectory
		//	2. create .doc2site subdirectory
		//	3. create config file
		//	4. create JSON control file
		//	5. create source directories
		//	6. create build directory
		//

		
		//	1. check for .doc2site subdirectory
		if (std::filesystem::exists(hidden)) {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : [{}] already intialized\n", working_dir.string());
			return EXIT_FAILURE;
		}

		//	2. create .doc2site subdirectory
		std::error_code ec;
		if (!std::filesystem::create_directories(working_dir / hidden / "cache", ec)) {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : cannot create [{}]\n", (working_dir / hidden).string());
			return EXIT_FAILURE;
		}

		//	3. create config file
		std::ofstream cfg((working_dir / cfg_file_name).string(), std::ios::trunc);
		if (cfg.is_open()) {
			//	write default values
			cfg << "#" << std::endl;
			cfg << "verbose\t= 2" << std::endl;
			cfg << "tag\t= " << boost::uuids::to_string(uuid_gen()) << std::endl;
			cfg << "[generator]" << std::endl;
			cfg << "include-path\t= " << (working_dir / subdir_src / "include").string() << std::endl;
			cfg << "build\t= " << (working_dir / subdir_build).string() << std::endl;
			cfg << "control\t= " << (working_dir / control_file_name).string() << std::endl;

			auto const loc = cyng::sys::get_system_locale();
			cfg << "locale\t= " << loc.at(cyng::sys::info::NAME) << std::endl;
			cfg << "country\t= " << loc.at(cyng::sys::info::COUNTRY) << std::endl;
			cfg << "language\t= " << loc.at(cyng::sys::info::LANGUAGE) << std::endl;
			cfg << "encoding\t= " << loc.at(cyng::sys::info::ENCODING) << std::endl;

		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : cannot create [{}]\n", (working_dir / cfg_file_name).string());
			return EXIT_FAILURE;
		}


		//	4. create JSON control file
		std::ofstream control((working_dir / control_file_name).string(), std::ios::trunc);
		if (control.is_open()) {
			//	write content
			auto const obj = cyng::make_object(generate_control());
			cyng::io::serialize_json(control, obj);
		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : cannot create [{}]\n", (working_dir / control_file_name).string());
			return EXIT_FAILURE;
		}
		
		//	5. create source directories
		if (!std::filesystem::create_directories(working_dir / subdir_src / "include" / "images", ec)) {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : cannot create [{}]: {}\n", (working_dir / subdir_src).string(), ec.message());
			return EXIT_FAILURE;
		}
		if (!std::filesystem::create_directories(working_dir / subdir_src / "css", ec)) {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : cannot create [{}]: {}\n", (working_dir / subdir_src / "css").string(), ec.message());
			return EXIT_FAILURE;
		}

		//	6. create build directory
		if (!std::filesystem::create_directories(working_dir / subdir_build, ec)) {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***warn  : cannot create [{}]: {}\n", (working_dir / subdir_build).string(), ec.message());
			return EXIT_FAILURE;
		}

		//	7. "home" docscript file
		std::ofstream home((working_dir / subdir_src / "home.docscript").string(), std::ios::trunc);
		if (home.is_open()) {
			home
				<< ".h1(home)"
				<< std::endl
				;
		}

		//	8. css file
		std::ofstream css((working_dir / subdir_src / "css" / "style.css").string(), std::ios::trunc);
		if (css.is_open()) {
			css
				<< "/* style.css */"
				<< std::endl
				;
		}

		return EXIT_SUCCESS;
	}

	cyng::tuple_t initialize::generate_control() const {
		return cyng::make_tuple(
			cyng::make_param("languages", cyng::make_vector({ "de", "en", "fr", "ru" })),
			cyng::make_param("pages", cyng::make_vector({ generate_page_home() })),
			cyng::make_param("downloads", cyng::make_vector({  })),
			cyng::make_param("menus", cyng::make_vector({ generate_menu_main()})),
			cyng::make_param("footers", cyng::make_vector({  })),
			cyng::make_param("site", cyng::make_tuple(
				cyng::make_param("name", "example.com"),
				cyng::make_param("index", "home"),
				cyng::make_param("css", "css/style.css"),
				cyng::make_param("pages", cyng::make_vector({ "home" }))
			))
		);
	}

	cyng::tuple_t initialize::generate_page_home() const {
		return cyng::make_tuple(
			cyng::make_param("name", "home"),
			cyng::make_param("title", "Home"),
			cyng::make_param("source", (subdir_src / "home.docscript").string()),
			cyng::make_param("enabled", true),
			cyng::make_param("menu", "main"),
			cyng::make_param("footer", "main")
		);
	}

	cyng::tuple_t initialize::generate_menu_main() const {
		std::filesystem::path const img  = "images";

		return cyng::make_tuple(
			cyng::make_param("name", "main"),
			cyng::make_param("placement", "sticky-top"),
			cyng::make_param("color-scheme", "light"),
			cyng::make_param("brand", (img / "logo.svg").string()),
			cyng::make_param("items", cyng::make_vector({ 

				cyng::make_tuple(
					cyng::make_param("title", "Home"),
					cyng::make_param("ref", "home"),
					cyng::make_param("enabled", true)
				)
			}))
		);
	}

}
