/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 

#include <boost/program_options.hpp>
#include <cyng/compatibility/file_system.hpp>
#include <boost/config.hpp>
#include <boost/predef.h>
#include <fstream>
#include <iostream>
#include <DOCC_project_info.h>
#include "site.h"
#if BOOST_OS_WINDOWS
#include <windows.h>
#endif

/**
 * Compiles all docScript files in the specified directory und uses the generated
 * meta data to compile a an index to navigate to all generated output files.
 *
 * Start with
 * @code
 * build/site -V9 ~/projects/docc/src/main/examples/site.json
 * build/site -V9 C:\projects\docc\src\main\examples\site.json
 * @endcode
 */
int main(int argc, char* argv[]) {

	try
	{
		const cyng::filesystem::path cwd = cyng::filesystem::current_path();

		std::string config_file;
		std::string cfg_file = (cwd / "docscript" / "site.json").string();
		std::string out_dir = (cwd / "build").string();

		//
		//	generic options
		//
		boost::program_options::options_description generic("Generic options");
		generic.add_options()

			("help,h", "print usage message")
			("version,v", "print version string")
			("build,b", "last built timestamp and platform")
			("config,C", boost::program_options::value<std::string>(&config_file)->default_value("batch.cfg"), "configuration file")

			;

		//
		//	all compiler options
		//
		boost::program_options::options_description compiler("compiler");
		compiler.add_options()

			("site,S", boost::program_options::value(&cfg_file)->default_value(cfg_file), "config file")
			("output,O", boost::program_options::value(&out_dir)->default_value(out_dir), "output directory")
			("include-path,I", boost::program_options::value< std::vector<std::string> >()->default_value(std::vector<std::string>(1, cwd.string()), cwd.string()), "include path")
			//	verbose level
			("verbose,V", boost::program_options::value<int>()->default_value(0)->implicit_value(1), "verbose level")
			("robot,R", boost::program_options::bool_switch()->default_value(true), "generate robots.txt")
			//	https://www.sitemaps.org/index.html
			("sitemap", boost::program_options::bool_switch()->default_value(false), "generate a sitemap file")
			;

		//
		//	all you can grab from the command line
		//
		boost::program_options::options_description cmdline_options;
		cmdline_options.add(generic).add(compiler);

		//
		//	positional arguments
		//
		boost::program_options::positional_options_description p;
		p.add("source", -1);

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
				<< "docScript site generator v"
				<< DOCC_VERSION
				<< std::endl
				;
			return EXIT_SUCCESS;
		}

		if (vm.count("build"))
		{
			std::cout
				<< "last built at : "
				<< DOCC_BUILD_DATE
				<< std::endl
				<< "Platform      : "
				<< DOCC_PLATFORM
				<< std::endl
				<< "Compiler      : "
				<< BOOST_COMPILER
				<< std::endl
				<< "StdLib        : "
				<< BOOST_STDLIB
				<< std::endl

#if defined(BOOST_LIBSTDCXX_VERSION)
				<< "StdLib++      : "
				<< BOOST_LIBSTDCXX_VERSION
				<< std::endl
#endif

				<< "BOOSTLib      : "
				<< BOOST_LIB_VERSION
				<< " ("
				<< BOOST_VERSION
				<< ")"
				<< std::endl

				<< "shared mutex  : "
#if defined(__CPP_SUPPORT_N4508)
				<< "no"
#else
				<< "yes"
#endif
				<< std::endl
				<< "uint8_t type  : "
#if defined(__CPP_SUPPORT_P0482R6)
				<< "yes"
#else
				<< "no"
#endif
				<< std::endl
				<< std::endl
				;
			return EXIT_SUCCESS;
		}


		std::ifstream ifs(config_file);
		if (!ifs)
		{
			std::cout
				<< "***info: config file: "
				<< config_file
				<< " not found"
				<< std::endl
				;
		}
		else
		{
			boost::program_options::store(boost::program_options::parse_config_file(ifs, compiler), vm);
			boost::program_options::notify(vm);
		}

		const int verbose = vm["verbose"].as< int >();
		if (verbose > 0)
		{
			std::cout
				<< "***info: verbose level: "
				<< verbose
				<< std::endl
				;

		}

		//
		//	read specified include paths
		//
		auto const inc_paths = vm["include-path"].as< std::vector<std::string>>();

		if (verbose > 0)
		{
			std::cout
				<< "Include paths are: "
				<< std::endl
				;
			std::copy(inc_paths.begin(), inc_paths.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
		}

#if BOOST_OS_WINDOWS
		//
		//	set console outpt code page to UTF-8
		//	requires a TrueType font like Lucida 
		//
		if (::SetConsoleOutputCP(65001) == 0)
		{
			std::cerr
				<< "Cannot set console code page"
				<< std::endl
				;

		}
#endif
		//
		//	Construct driver instance
		//
  		docscript::site s(inc_paths, verbose);

		//
		//	Start driver with the main/input file
		//
		return s.run(cfg_file
			, out_dir
			, vm["robot"].as< bool >()
			, vm["sitemap"].as< bool >());


	}
	catch (std::exception& e)
	{
		std::cerr
			<< e.what()
			<< std::endl
			;
	}

	return EXIT_FAILURE;
		
}
