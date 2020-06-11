/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 

#ifndef DOCC_SITE_H
#define DOCC_SITE_H

#include <cyng/object.h>
#include <cyng/intrinsics/sets.h>
#include <chrono>
#include <cyng/compatibility/file_system.hpp>

namespace docscript
{

	/**
	 * Takes a site description and generated a complete site from
	 * all sources as specified.
	 */
	class site
	{
		using chrono_idx_t = std::map<std::chrono::system_clock::time_point, cyng::param_map_t>;

	public:
		/**
		 * Constructor
		 *
		 * @param inc vector of include paths
		 * @param verbose verbose level. The higher the number, the more will be logged.
		 */
		site(std::vector< std::string >const& inc, int verbose);
		virtual ~site();

		/**
		 * In a first step the compiler generates an intermediate file from all
		 * input files that contains instructions to generate the output file.
		 * In a second step a special VM executes the instructions to generate the
		 * requested output file (HTML, PDF, ...)
		 *
		 * @param cfg config file (site.json)
		 * @param out build directory
		 */
		int run(cyng::filesystem::path const& cfg
			, cyng::filesystem::path const& out
			, bool gen_robot
			, bool gen_sitemap);

	private:

	private:
		/**
		 * Manage a list of include directories.
		 * If the driver opens a file it searches in all given directories
		 * until the spcified file is found.
		 */
		std::vector< std::string > const includes_;

		/**
		 * verbosity level.
		 * 0 == print only errors
		 */
		int const verbose_;


		//std::map<cyng::filesystem::path, cyng::param_map_t> index_;
	};

}

#endif
