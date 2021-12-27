
#include <version.hpp>
#include <initialize.h>
#include <build.h>
#include <site_defs.h>
#include <docc/utils.h>

#include <cyng/sys/locale.h>

#include <iostream>
#include <fstream>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/predef.h>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

#include <fmt/core.h>
#include <fmt/color.h>


int main(int argc, char* argv[]) {

	std::string config_file = std::string("doc2site-") + std::string(docc::version_string) + ".cfg";
	auto const here = std::filesystem::current_path();
	std::string cmd = "build";
	std::string stag = "3d17ff49-a752-41d3-a1b0-b9c9aa11952b";  //  stable toc IDs
	std::size_t pool_size = std::min<std::size_t>(std::thread::hardware_concurrency(), 4) * 2ul;
	auto out_dir = (here / docscript::subdir_out).string();
	auto cache_dir = (here / docscript::hidden / "cache").string();
	auto bs_dir = (here / docscript::hidden / "bootstrap" / docscript::bootstrap_version).string();
	auto control_file = (here / docscript::control_file_name).string();

	auto const loc = cyng::sys::get_system_locale();
	auto def_locale = loc.at(cyng::sys::info::NAME);
	auto def_country = loc.at(cyng::sys::info::COUNTRY);
	auto def_lang = loc.at(cyng::sys::info::LANGUAGE);
	auto def_enc = loc.at(cyng::sys::info::ENCODING);

	//
	//	generic options
	//
	boost::program_options::options_description generic("Generic options");
	generic.add_options()

		("help,h", "print usage message")
		("version,v", "print version string")
		("config,C", boost::program_options::value<std::string>(&config_file)->default_value(config_file), "configuration file")
		;

	//
	//	all runtime options
	//
	boost::program_options::options_description general("general");
	general.add_options()

		("command,c", boost::program_options::value(&cmd)->default_value(cmd), "command")
		("tag,T", boost::program_options::value(&stag)->default_value(stag), "VM tag")
		//	verbose level
		("verbose,V", boost::program_options::value<int>()->default_value(0)->implicit_value(1), "verbose level")
		;


	boost::program_options::options_description gen("generator");
	gen.add_options()
		("generator.include-path,I", boost::program_options::value< std::vector<std::string> >()->default_value(std::vector<std::string>(1, here.string()), here.string()), "include paths")
		("generator.out", boost::program_options::value(&out_dir)->default_value(out_dir), "output directory")
		("generator.cache", boost::program_options::value(&cache_dir)->default_value(cache_dir), "cache for temporary files")
		("generator.bs", boost::program_options::value(&bs_dir)->default_value(bs_dir), "local bootstrap sources")
		("generator.control", boost::program_options::value(&control_file)->default_value(control_file), "control file")
		("generator.locale", boost::program_options::value(&def_locale)->default_value(def_locale), "locale")
		("generator.country", boost::program_options::value(&def_country)->default_value(def_country), "country")
		("generator.language", boost::program_options::value(&def_lang)->default_value(def_lang), "language")
		("generator.encoding", boost::program_options::value(&def_enc)->default_value(def_enc), "encoding")
		;

	//
	//	all you can grab from the command line
	//
	boost::program_options::options_description cmdline_options;
	cmdline_options.add(generic).add(general).add(gen);

	//
	//	positional arguments
	//
	boost::program_options::positional_options_description p;
	p.add("command", -1);

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
	boost::program_options::notify(vm);

	if (vm.count("help"))
	{
		std::cout
			<< cmdline_options
			<< std::endl
			;
		return EXIT_SUCCESS;
	}

	if (vm.count("version"))
	{
		std::cout
			<< "doc2site generator v"
			<< docc::version_string
			<< std::endl
			;
		return EXIT_SUCCESS;
	}

	docscript::init_console();

	std::ifstream ifs(config_file);
	if (!ifs)
	{
		fmt::print(
			stdout,
			fg(fmt::color::dark_orange) | fmt::emphasis::bold,
			"***warn  : config file [{}] not found\n", config_file);
	}
	else
	{
		//
		//
		// 
		// 
		// 
		// 
		// 	options available from config file
		//
		boost::program_options::options_description file_options;
		file_options.add_options()
			("pool-size,P", boost::program_options::value(&pool_size)->default_value(pool_size), "Thread pool size")
			;
		file_options.add(general).add(gen);

		boost::program_options::store(boost::program_options::parse_config_file(ifs, file_options), vm);
		boost::program_options::notify(vm);
	}

	//
	//  verbose level
	//
	auto const verbose = vm["verbose"].as< int >();

	fmt::print(
		stdout,
		fg(fmt::color::gray),
		"***info : verbose level = [{}]\n", verbose);

	//
	//	read specified include paths
	//
	auto const inc_paths = docscript::get_include_paths(
		vm["generator.include-path"].as< std::vector<std::string>>(),
		here
	);

	if (verbose > 1) {
		fmt::print(
			stdout,
			fg(fmt::color::gray),
			"***info : {} include paths\n", inc_paths.size());

		std::copy(inc_paths.begin(), inc_paths.end(), std::ostream_iterator<std::filesystem::path>(std::cout, "\n"));
	}

	//
	//	get VM tag
	//
	auto const tag = boost::uuids::string_generator()(stag);

	//
	//  start 
	//

	std::cout << cmd << std::endl;
	if (boost::algorithm::equals(cmd, "init")) {

		docscript::initialize init(inc_paths, verbose);
		return init.run(here);
	}
	else if (boost::algorithm::equals(cmd, "build")) {

		docscript::build build(inc_paths, out_dir, cache_dir, bs_dir, verbose, def_locale, def_country, def_lang, def_enc);
		return build.run(control_file);
	}

	fmt::print(
		stderr,
		fg(fmt::color::crimson) | fmt::emphasis::bold,
		"***warn  : unknown command [{}]\n", cmd);

	return EXIT_FAILURE;
}