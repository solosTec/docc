/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <boost/program_options.hpp>
#include <cyng/compatibility/file_system.hpp>
#include <boost/config.hpp>
#include <boost/predef.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <DOCC_project_info.h>
#include <CYNG_project_info.h>
#include <NODE_project_info.h>
#include "controller.h"
#if BOOST_OS_WINDOWS
#include <windows.h>
#endif
#if BOOST_OS_LINUX
#include <sys/resource.h>
#endif

/**
 * main entry point.
 * To run as windows service additional preparations
 * required.
 */
int main(int argc, char **argv) {

	try {
		//	will contain the path to an optional configuration file
		std::string config_file;

		//
		//	generic options
		//
		boost::program_options::options_description generic("Generic options");
		generic.add_options()

			("help,h", "print usage message")
			("version,v", "print version string")
			("build,b", "last built timestamp and platform")
			("config,C", boost::program_options::value<std::string>(&config_file)->default_value("plog.cfg"), "specify the configuration file")
			//	json, XML
			("default,D", boost::program_options::value<std::string>()->default_value("")->implicit_value("json"), "generate a default configuration and exit. options are json and XML")
			("ip,N", boost::program_options::bool_switch()->default_value(false), "show local IP address and exit")
			//("show,s", boost::program_options::bool_switch()->default_value(false), "show configuration")
			("console", boost::program_options::bool_switch()->default_value(false), "log (only) to console")

			;

		//	get the working directory
		const cyng::filesystem::path cwd = cyng::filesystem::current_path();

		//	path to JSON configuration file
		std::string json_path;
		unsigned int pool_size = 1;

#if BOOST_OS_LINUX
		struct rlimit rl;
		int rc = ::getrlimit(RLIMIT_NOFILE, &rl);
		BOOST_ASSERT_MSG(rc == 0, "getrlimit() failed");
#endif    

		//
		//	plog node options
		//
		boost::program_options::options_description plog_options("plog");
		plog_options.add_options()

			("plog.json,J", boost::program_options::value<std::string>(&json_path)->default_value((cwd / "plog.json").string()), "JSON configuration file")
			("plog.pool-size,P", boost::program_options::value<unsigned int>(&pool_size)->default_value(std::thread::hardware_concurrency() * 2), "Thread pool size")

			("include-path,I", boost::program_options::value< std::vector<std::string> >()->default_value(std::vector<std::string>(1, cwd.string()), cwd.string()), "include path")
			("verbose,V", boost::program_options::value<int>()->default_value(0)->implicit_value(1), "verbose level")


#if BOOST_OS_WINDOWS
			("service.enabled,S", boost::program_options::value<bool>()->default_value(false), "run as NT service")
			("service.name", boost::program_options::value< std::string >()->default_value(std::string("plog_server") + std::string(DOCC_SUFFIX)), "NT service name")
#elif BOOST_OS_LINUX
			("RLIMIT_NOFILE.soft", boost::program_options::value< rlim_t >()->default_value(rl.rlim_cur), "RLIMIT_NOFILE soft")
			("RLIMIT_NOFILE.hard", boost::program_options::value< rlim_t >()->default_value(rl.rlim_max), "RLIMIT_NOFILE hard")
#endif
			;

		//
		//	all you can grab from the command line
		//
		boost::program_options::options_description cmdline_options;
		cmdline_options.add(generic).add(plog_options);

		//
		//	read all data
		//
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
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
				<< "plog server v"
				<< DOCC_VERSION
				<< std::endl
				<< "Copyright (C) 2012-2019 S. Olzscher. All rights reserved."
				<< std::endl
				;
			return EXIT_SUCCESS;
		}

		if (vm.count("build"))
		{
			std::cout
				<< "configured at: "
				<< DOCC_BUILD_DATE
				<< " UTC"
				<< std::endl
				<< "Platform     : "
				<< DOCC_PLATFORM
				<< std::endl
				<< "Compiler     : "
				<< BOOST_COMPILER
				<< " (__cplusplus "
				<< __cplusplus
				<< ")"
				<< std::endl
				<< "StdLib       : "
				<< BOOST_STDLIB
				<< std::endl
				<< "BOOSTLib     : v"
				<< BOOST_LIB_VERSION
				<< " ("
				<< BOOST_VERSION
				<< ")"
				<< std::endl
				<< "OpenSSL      : v"
				<< DOCC_SSL_VERSION
				<< std::endl
				<< "build type   : "
#if BOOST_OS_WINDOWS
				<< CMAKE_INTDIR
#else
				<< DOCC_BUILD_TYPE
#endif
				<< std::endl
				<< "CYNG library : v"
				<< CYNG_VERSION
				<< std::endl
				<< "NODE library : v"
				<< NODE_VERSION
				<< std::endl
				<< "CPU count    : "
				<< std::thread::hardware_concurrency()
				<< std::endl
				;
			return EXIT_SUCCESS;
		}

		//	read parameters from config file
		const std::string cfg = vm["config"].as< std::string >();
		std::ifstream ifs(cfg);
		if (ifs.is_open())
		{
			//
			//	options available from config file
			//
			boost::program_options::options_description file_options;
			file_options.add(plog_options);//.add(authopt);

			//	read parameters from config file
			boost::program_options::store(boost::program_options::parse_config_file(ifs, file_options), vm);

			//	update local values
			boost::program_options::notify(vm);

		}

		const int verbose = vm["verbose"].as< int >();
		if (verbose != 0)
		{
			std::cout
				<< "***info: verbose level: "
				<< verbose
				<< std::endl
				;
		}

		//
		//	"include-path""
		//
		auto includes = vm["include-path"].as< std::vector<std::string> >();
		includes.push_back(cyng::filesystem::temp_directory_path().string());
		includes.push_back("");

		if (verbose > 0)
		{
			std::cout
				<< "Include paths are: "
				;
			std::copy(includes.begin(), includes.end(), std::ostream_iterator<std::string>(std::cout, "\n "));
			std::cout
				<< std::endl
				;
		}

		//
		//	create a controller object
		//
		plog::controller ctrl(pool_size, json_path);

		const std::string config_type = vm["default"].as<std::string>();
		if (!config_type.empty())
		{
			//	write default configuration
			return ctrl.create_config(config_type);
		}

	    //if (vm["ip"].as< bool >())
	    //{
	    //    //	show local IP adresses
	    //    return ctrl.show_ip();
	    //}

	    //if (vm["show"].as< bool >())
	    //{
	    //    //	show configuration
	    //    return ctrl.show_config();
	    //}


#if BOOST_OS_WINDOWS
		if (vm["service.enabled"].as< bool >())
		{
			//	run as service 
			const std::string srv_name = vm["service.name"].as< std::string >();
			::OutputDebugString(srv_name.c_str());
			//return ctrl.run_as_service(std::move(ctrl), srv_name);
		}
#endif

#if BOOST_OS_LINUX
		rc = ::setrlimit(RLIMIT_NOFILE, &rl);
		BOOST_ASSERT_MSG(rc == 0, "setrlimit() failed");
#endif

		BOOST_ASSERT_MSG(pool_size != 0, "empty thread pool");
		return ctrl.run(vm["console"].as< bool >());
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
