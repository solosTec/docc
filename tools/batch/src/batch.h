/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCC_BATCH_H
#define DOCC_BATCH_H

#include <cyng/object.h>
#include <cyng/intrinsics/sets.h>
#include <chrono>
#include <boost/filesystem.hpp>

namespace docscript
{

	/**
	 * Compiles all docScript files ti HTML stubs in the specified directory und uses the generated
	 * meta data to compile a an index to navigate to all generated output files.
	 */
	class batch
	{
		using chrono_idx_t = std::map<std::chrono::system_clock::time_point, cyng::object>;

	public:
		/**
		 * Constructor
		 *
		 * @param inc vector of include paths
		 * @param verbose verbose level. The higher the number, the more will be logged.
		 */
		batch(std::vector< std::string >const& inc, int verbose);
		virtual ~batch();

		/**
		 * In a first step the compiler generates an intermediate file from all
		 * input files that contains instructions to generate the output file.
		 * In a second step a special VM executes the instructions to generate the
		 * requested output file (HTML, PDF, ...)
		 *
		 * @param master master file
		 * @param tmp temporary intermediate file.
		 * @param out output file (html)
		 */
		int run(boost::filesystem::path const& inp
			, boost::filesystem::path const& out
			, bool gen_robot
			, bool gen_sitemap);

	private:
		void process_file(boost::filesystem::path const& inp
			, boost::filesystem::path const& out);

		void generate_index(boost::filesystem::path const& out
			, bool gen_robot
			, bool gen_sitemap);

		void generate_index_page(boost::filesystem::path const& out, chrono_idx_t const& idx);
		void generate_index_map(boost::filesystem::path const& out, chrono_idx_t const& idx);
		void generate_robots_txt(boost::filesystem::path const& out, chrono_idx_t const& idx, bool);
		void generate_sitemap(boost::filesystem::path const& out, chrono_idx_t const& idx);

		chrono_idx_t get_sorted();

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


		cyng::param_map_t index_;
	};

}

#endif
